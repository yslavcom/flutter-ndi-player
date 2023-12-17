/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:53:53  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:53:53 . All rights reserved */

#include "player.hpp"
#include "common/logger.hpp"
#include "audio-play/include/audio_play_lib.h"

#ifdef ANDROID_PLATFORM
#include <android_native_app_glue.h>
#endif

#include <type_traits>


// #define _DBG_PLAYER
#ifdef _DBG_PLAYER
    #define DBG_PLAYER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PLAYER(format, ...)
#endif

Player::Player()
    : mRenderVidFrameObserver(nullptr)
    , mVideoDecoder(nullptr)
    , mAudioInitialised(false)
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

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    DBG_PLAYER("Player::onFrame, frame:%p, renderObs:%p, mVideoDecoder:%p(ready:%d)\n",
        frame, mRenderVidFrameObserver, mVideoDecoder, (mVideoDecoder ? mVideoDecoder->isReady(): false));

    if (!frame)
    {
        DBG_PLAYER("Missing frame\n");
        return;
    }

    if (!mRenderVidFrameObserver)
    {
        DBG_PLAYER("Missing frame observer\n");
        return;
    }
    // Apple test pattern generator sends frame just once when the dimensions are still unknown.
    // It should be resolved after refactoring the GUI
    auto [xRes, yRes] = mRenderVidFrameObserver->getOutDim();
    if (!xRes || !yRes)
    {
        DBG_PLAYER("Bad dimensions: %d/%d\n", xRes, yRes);
    }
    // render the frame
    if (xRes && yRes)
    {
        auto x = xRes;
        auto y = yRes;

        std::visit(FrameQueue::overloaded {
            [this, x, y](FrameQueue::VideoFrameStr& arg)
            {
                // uncompressed frame
                size_t size = 0;
                auto scaledFrame = convScaleFrame(arg, x, y, size);
                DBG_PLAYER("Render uncompressed\n");
                mRenderVidFrameObserver->onRender(std::move(scaledFrame), size);
            },
            [this, cleanupCb = frame->second](FrameQueue::VideoFrameCompressedStr& arg)
            {
                // compressed frame, must be cleaned up after decoding
                DBG_PLAYER("Compressed, x:%d, y:%d\n", arg.xres, arg.yres);

                std::lock_guard lk(mDecoderMu);
                if (mVideoDecoder)
                {
                    if (!mVideoDecoder->isReady())
                    {
                        DBG_PLAYER("Request render/decode window setup\n");
                        mVideoDecoder->setDimensions(arg.xres, arg.yres);
                        mVideoDecoder->setCodecFourCC(H26x::FourCC{arg.fourCC});
                        mVideoDecoder->requestSetup();
                    }
                    auto res = mVideoDecoder->pushToDecode(arg, cleanupCb);
                    DBG_PLAYER("mVideoDecoder->pushToDecode:%d, rate:%d/%d\n", res, arg.frameRateN, arg.frameRateD);
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

    auto& audio = frame->first;
    if (!mAudioInitialised)
    {
        auto cb = frame->second.target<void(void*)>();
        audio_setup(cb);
        mAudioInitialised = true;
    }

    audio_push_aud_frame(reinterpret_cast<uintptr_t>(audio.opaque), audio.chanNo, reinterpret_cast<uintptr_t>(audio.samples), audio.samplesNo, audio.stride, audio.planar);

#if 0
//    DBG_PLAYER("dump audio, remaining:%d\n", remainingCount);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
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

