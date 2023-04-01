#include "ndi-app.hpp"

#include "common/logger.hpp"

#include <iostream>
#include <type_traits>

NdiApp::NdiApp()
{
}

NdiApp::~NdiApp()
{
}


bool NdiApp::createReceiver(const std::string& name, const std::string& url, Quality quality)
{
    const NDIlib_source_t ndiSources(name.c_str(), url.c_str());

    // TODO: use NDIlib_recv_color_format_compressed_v5 to pass compressed video through without automatic decompression
    NDIlib_recv_create_v3_t recvDescHi{};
    recvDescHi.color_format = NDIlib_recv_color_format_fastest;
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
    NDIlib_video_frame_v2_t* video = (NDIlib_video_frame_v2_t*)malloc(sizeof(NDIlib_video_frame_v2_t));
    NDIlib_audio_frame_v3_t* audio = (NDIlib_audio_frame_v3_t*)malloc(sizeof(NDIlib_audio_frame_v3_t));
    NDIlib_metadata_frame_t* meta = (NDIlib_metadata_frame_t*)malloc(sizeof(NDIlib_metadata_frame_t));

    NDIlib_frame_type_e ret = NDIlib_recv_capture_v3(rxInst->src(), video, audio, meta, 1000);

    switch (ret)
    {
        case NDIlib_frame_type_none:
            free(video);
            free(audio);
            free(meta);
            break;

        case NDIlib_frame_type_video:
        {
            if (video->p_data)
            {
                // push video to queue after checking for audio !!!
                auto releaseCb = [this, rxInst](void* userData)
                {
                    // rxInst is owned by class object
                    if (!userData) { return; }
                    auto video = (NDIlib_video_frame_v2_t*)userData;
                    NDIlib_recv_free_video_v2(rxInst->src(), video);
                    free(video);
                };
                receivedPack(video, releaseCb);
                free(audio);
            }
            else
            {
                NDIlib_recv_free_video_v2(rxInst->src(), video);
                free(video);
                free(audio);
            }
            free(meta);
            break;
        }

        case NDIlib_frame_type_audio:
        {
            auto releaseCb = [this, rxInst](void* userData)
            {
                // rxInst is owned by class object
                if (!userData) { return; }
                auto audio = (NDIlib_audio_frame_v3_t*)userData;
                NDIlib_recv_free_audio_v3(rxInst->src(), audio);
                free(audio);
            };
            receivedPack(audio, releaseCb);
            free(video);
            free(meta);
            break;
        }

        case NDIlib_frame_type_metadata:
            NDIlib_recv_free_metadata(rxInst->src(), meta);
            free(video);
            free(audio);
            free(meta);
            break;

        case NDIlib_frame_type_error:
            LOGE("NDI pack rx error\n");
            break;

        default:
            free(video);
            free(audio);
            free(meta);
            break;
    }
    return true;
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

template <typename T>
void NdiApp::receivedPack(T *pack, std::function<void(void* userData)> releaseCb)
{
    if constexpr (std::is_same_v<T, NDIlib_video_frame_v2_t>)
    {
        for (auto& el: mInputPacketsObservers)
        {
            el->receivedVideoPack(pack, releaseCb);
        }
    }
    else if constexpr (std::is_same_v<T, NDIlib_audio_frame_v3_t>)
    {
        for (auto& el: mInputPacketsObservers)
        {
            el->receivedAudioPack(pack, releaseCb);
        }
    }
}

