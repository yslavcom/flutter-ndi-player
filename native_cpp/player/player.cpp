/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:53:53  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:53:53 . All rights reserved */

#include "player.hpp"
#include "common/logger.hpp"
#include "audio-play/include/audio_play_lib.h"

#ifdef ANDROID_PLATFORM
#include <android_native_app_glue.h>
#endif

#include <type_traits>


// #define _DBG_PLAYER
// #define _DBG_PLAYER_AUD
// #define _DBG_PLAYER_VID

#ifdef _DBG_PLAYER
    #define DBG_PLAYER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PLAYER(format, ...)
#endif

#ifdef _DBG_PLAYER_AUD
    #define DBG_PLAYER_AUD(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PLAYER_AUD(format, ...)
#endif

#ifdef _DBG_PLAYER_VID
    #define DBG_PLAYER_VID(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PLAYER_VID(format, ...)
#endif

Player::Player()
    : mRenderVidFrameObserver(nullptr)
    , mVideoDecoder(nullptr)
    , mAudioInitialised(false)
    , mState(State::Idle)
#if 0
    , mNalParse{}
#endif
{
}

Player::~Player()
{
}

void Player::setRenderObserver(RenderVidFrameObserver* obs)
{
    mRenderVidFrameObserver = obs;
}

void Player::setDecoder(Video::Decoder* decoder)
{
    std::lock_guard lk(mDecoderMu);
    mVideoDecoder = decoder;
}

void Player::reStart()
{
    std::lock_guard lk(mDecoderMu);
    mState = State::Connecting;
}

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    DBG_PLAYER_VID("Player::onFrame, frame:%p, renderObs:%p, mVideoDecoder:%p(ready:%d)\n",
        frame, mRenderVidFrameObserver, mVideoDecoder, (mVideoDecoder ? mVideoDecoder->isReady(): false));

    if (!frame)
    {
        DBG_PLAYER_VID("Missing frame\n");
        return;
    }

    if (!mRenderVidFrameObserver)
    {
        DBG_PLAYER_VID("Missing frame observer\n");
        return;
    }
    // Apple test pattern generator sends frame just once when the dimensions are still unknown.
    // It should be resolved after refactoring the GUI
    auto [xRes, yRes] = mRenderVidFrameObserver->getOutDim();
    if (!xRes || !yRes)
    {
        DBG_PLAYER_VID("Bad dimensions: %d/%d\n", xRes, yRes);
    }
    // render the frame
    if (xRes && yRes)
    {
        auto x = xRes;
        auto y = yRes;

        std::visit(FrameQueue::overloaded {
            [this, x, y](FrameQueue::VideoFrameStr& arg)
            {
                DBG_PLAYER_VID("Uncompressed, x:%d, y:%d\n", arg.xres, arg.yres);

                // uncompressed frame
                size_t size = 0;
                auto scaledFrame = convScaleFrame(arg, x, y, size);
                DBG_PLAYER_VID("Render uncompressed\n");
                mRenderVidFrameObserver->onRender(std::move(scaledFrame), size);
            },
            [this, cleanupCb = frame->second](FrameQueue::VideoFrameCompressedStr& arg)
            {
                auto sendToDecode = [this, cleanupCb](FrameQueue::VideoFrameCompressedStr& frm, auto fourCC){
                    if (mVideoDecoder)
                    {
                        if (!mVideoDecoder->isReady())
                        {
                            DBG_PLAYER_VID("Request render/decode window setup\n");
                            mVideoDecoder->setDimensions(frm.xres, frm.yres);
                            mVideoDecoder->setCodecFourCC(fourCC);
                            mVideoDecoder->requestSetup();
                        }
                        auto res = mVideoDecoder->pushToDecode(frm, cleanupCb);
                        DBG_PLAYER_VID("mVideoDecoder->pushToDecode:%d, rate:%d/%d\n", res, frm.frameRateN, frm.frameRateD);
                    }
                };

                // compressed frame, must be cleaned up after decoding
                DBG_PLAYER_VID("Compressed, x:%d, y:%d\n", arg.xres, arg.yres);

                std::lock_guard lk(mDecoderMu);

                switch (mState)
                {
                case State::Idle:
                break;

                case State::Connecting:
                    if (arg.isKeyFrame)
                    {
                        mState = State::Connected;
                        sendToDecode(arg, H26x::FourCC{arg.fourCC});
                    }
                break;

                case State::Connected:
                    sendToDecode(arg, H26x::FourCC{arg.fourCC});
                break;
                }
            }
        }, frame->first);
    }
}

void Player::onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }
#if 1
    auto& audio = frame->first;
    if (!mAudioInitialised)
    {
        auto cb = frame->second.first;
        auto ctxt = frame->second.second;
        DBG_PLAYER_AUD("Set Callback:%p, chanNo:%d, samples:%p, samplesNo:%d, stride:%d, planar:%d\n",
            cb, frame->first.chanNo, frame->first.samples, frame->first.samplesNo, frame->first.stride, frame->first.planar);
        audio_setup(cb, reinterpret_cast<uintptr_t>(ctxt));
        mAudioInitialised = true;
    }

    audio_push_aud_frame(reinterpret_cast<uintptr_t>(audio.opaque), audio.chanNo, reinterpret_cast<uintptr_t>(audio.samples), audio.samplesNo, audio.stride, audio.planar);

#else
//    DBG_PLAYER("dump audio, remaining:%d\n", remainingCount);
    if (frame->second.first)
    {
        frame->second.first(frame->second.second, frame->first.opaque);
    }
#endif
}

std::unique_ptr<uint8_t[]> Player::convScaleFrame(const FrameQueue::VideoFrameStr& frame, unsigned xRes, unsigned yRes, size_t& size)
{
    const int outputPixelComponentscount = 4; //RGBA

    size = xRes * yRes * outputPixelComponentscount; // RGBA
    auto rgbaFrame = std::make_unique<uint8_t[]>(size);
    if (!rgbaFrame)
    {
        LOGE("Failed to allocate memory for rgba frame\n");
        return nullptr;
    }

    if (!mConvertScale)
    {
        mConvertScale = std::make_unique<ConvertScale>(frame.xres, frame.yres, xRes, yRes);
    }

    uint8_t* src[3] = {frame.data, nullptr, nullptr};
    int bytes_in_line = (0 == frame.stride) ? frame.xres : frame.stride;
    int srcLineSize[4] = {bytes_in_line, bytes_in_line, bytes_in_line, bytes_in_line};

    uint8_t* dest[4] = {rgbaFrame.get(), nullptr, nullptr, nullptr};
    auto lineSize = static_cast<int>(xRes) * outputPixelComponentscount;
    int destLineSize[4] = {lineSize, lineSize, lineSize, lineSize};

    if (0 < mConvertScale->scale(xRes, yRes, frame.xres, frame.yres, src, srcLineSize, dest, destLineSize))
    {
        return rgbaFrame;
    }
    LOGE("scale faild, size:%d, xRes:%d, yRes:%d, frame.xres:%d, frame.yres:%d, frame.stride:%d, bytes_in_line:%d\n",
        size, xRes, yRes, frame.xres, frame.yres, frame.stride, bytes_in_line);
    return nullptr;
}

