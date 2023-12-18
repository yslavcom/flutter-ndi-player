#include "ndi-app.hpp"

// #include <Processing.NDI.Lib.h>
#include <Processing.NDI.Advanced.h>

#include "common/logger.hpp"

#include <iostream>
#include <type_traits>
#include <cassert>


// #define _DBG_NDI_APP
#define _DBG_AUD_RX

#ifdef _DBG_NDI_APP
    #define DBG_NDI_APP(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_NDI_APP(format, ...)
#endif

#ifdef _DBG_AUD_RX
    #define DBG_AUD_RX(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_AUD_RX(format, ...)
#endif

NdiApp::NdiApp()
    : mUncompressedShq(false)
{
}

NdiApp::~NdiApp()
{
}

void NdiApp::requestKeyFrame()
{
    if (mRecvInst)
    {
        DBG_NDI_APP("Request key frame\n");
        NDIlib_recv_request_keyframe(mRecvInst->src());
    }
}

bool NdiApp::createReceiver(const std::string& name, const std::string& url, Quality quality)
{
    const NDIlib_source_t ndiSources(name.c_str(), url.c_str());

    // TODO: use NDIlib_recv_color_format_compressed_v5 to pass compressed video through without automatic decompression
    NDIlib_recv_create_v3_t recvDescHi{};

    if (mUncompressedShq)
    {
        // SHQ, decompressed
        recvDescHi.color_format = NDIlib_recv_color_format_fastest;
    }
    else
    {
        // Allow SpeedHQ frames, compressed H.264 frames, HEVC frames and HEVC/H264 with alpha, along with
        // compressed audio frames and OPUS support.
        recvDescHi.color_format = (NDIlib_recv_color_format_e)NDIlib_recv_color_format_ex_compressed_v5_with_audio;
    }

    recvDescHi.bandwidth = (quality == Quality::High) ? NDIlib_recv_bandwidth_highest : NDIlib_recv_bandwidth_lowest;
    recvDescHi.allow_video_fields = true;

    mRecvInst.reset();
    std::shared_ptr<RecvClass> rxInst = std::make_shared<RecvClass>(recvDescHi);
    if (!rxInst)
    {
        return false;
    }
    mRecvInst = rxInst;

    // Connect to our sources
    NDIlib_recv_connect(mRecvInst->src(), &ndiSources);

    return true;
}

bool NdiApp::stopReceiver()
{
    mRecvInst = nullptr;
    return true;
}

bool NdiApp::capturePackets()
{
    if (!mRecvInst)
    {
        return false;
    }

    return captureBlock(mRecvInst);
}

bool NdiApp::captureBlock(std::shared_ptr<RecvClass> rxInst)
{
    //NDIlib_video_frame_v2_t* video = (NDIlib_video_frame_v2_t*)malloc(sizeof(NDIlib_video_frame_v2_t));

    std::unique_ptr<NDIlib_video_frame_v2_t> video = std::make_unique<NDIlib_video_frame_v2_t>();
    std::unique_ptr<NDIlib_audio_frame_v3_t> audio = std::make_unique<NDIlib_audio_frame_v3_t>();
    std::unique_ptr<NDIlib_metadata_frame_t> meta = std::make_unique<NDIlib_metadata_frame_t>();

    NDIlib_frame_type_e ret = NDIlib_recv_capture_v3(rxInst->src(), video.get(), audio.get(), meta.get(), 1000);

    switch (ret)
    {
        case NDIlib_frame_type_none:
            break;

        case NDIlib_frame_type_video:
        {
            if (video->p_data)
            {
                // push video to queue after checking for audio !!!
                auto releaseCb = [this, rxInst](void* userData)
                {
                    // rxInst is a shred pointer and is owned by class object
                    if (!userData) { return; }
                    auto video = (NDIlib_video_frame_v2_t*)userData;
                    NDIlib_recv_free_video_v2(rxInst->src(), video);
                    delete(video);
                };
                receivedPack(std::move(video), releaseCb);
            }
            else
            {
                NDIlib_recv_free_video_v2(rxInst->src(), video.get());
            }
            break;
        }

        case NDIlib_frame_type_audio:
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<float, std::milli>(now - mTimeRefr);
            mTimeRefr = now;

            DBG_AUD_RX("Aud elapsed:%3.5f ms | audio:%p, inst:%p, inst_count:%d\n", elapsed.count(), audio.get(), rxInst->src(), rxInst.use_count());

            {
                std::lock_guard lk(mMutex);
                mAudRevMap[audio.get()] = rxInst;
            }
            receivedPack(std::move(audio), NdiApp::releaseAudioSampleS, this);
            break;
        }

        case NDIlib_frame_type_metadata:
            NDIlib_recv_free_metadata(rxInst->src(), meta.get());
            break;

        case NDIlib_frame_type_error:
            LOGE("NDI pack rx error\n");
            break;

        default:
            break;
    }
    return true;
}

void NdiApp::releaseAudioSample(void* releaseData)
{
    // rxInst is owned by class object
    if (!releaseData) { return; }

    std::lock_guard lk(mMutex);

    auto audio = (NDIlib_audio_frame_v3_t*)releaseData;
    auto rxInst = mAudRevMap.find(audio);
    assert(rxInst != mAudRevMap.cend() && "Failed to match the recevier pointer to the audio packet");
    NDIlib_recv_free_audio_v3(rxInst->second->src(), audio);
    DBG_AUD_RX("Clean aud, audio:%p, inst:%p, inst_count:%d\n", audio, rxInst->second->src(), rxInst->second.use_count());
    mAudRevMap.erase(rxInst);

    delete(audio);
}

void NdiApp::releaseAudioSampleS(void* context, void* releaseData)
{
    if (context)
    {
        reinterpret_cast<NdiApp*>(context)->releaseAudioSample(releaseData);
    }
}

void NdiApp::addObserver(InputPacketsObserver* obs)
{
    if (obs)
    {
        auto iter = mInputPacketsObservers.find(obs);
        if (iter == mInputPacketsObservers.cend())
        {
            mInputPacketsObservers.emplace(obs);
        }
    }
}

void NdiApp::removeObserver(InputPacketsObserver* obs)
{
    if (obs)
    {
        auto iter = mInputPacketsObservers.find(obs);
        if (iter != mInputPacketsObservers.cend())
        {
            mInputPacketsObservers.erase(iter);
        }
    }
}

template<typename T, typename C, typename... Args>
void NdiApp::receivedPack(std::unique_ptr<T> pack, C releaseCb, Args... args)
{
    if constexpr (std::is_same_v<T, NDIlib_video_frame_v2_t>)
    {
        for (auto& el: mInputPacketsObservers)
        {
            el->receivedVideoPack(std::move(pack), releaseCb);
        }
    }
    else if constexpr (std::is_same_v<T, NDIlib_audio_frame_v3_t>)
    {
        for (auto& el: mInputPacketsObservers)
        {
            el->receivedAudioPack(std::move(pack), releaseCb, args...);
        }
    }
}

