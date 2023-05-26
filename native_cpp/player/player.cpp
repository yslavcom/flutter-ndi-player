/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:53:53  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:53:53 . All rights reserved */

#include "player.hpp"
#include "common/logger.hpp"

#include <android_native_app_glue.h>

#include <type_traits>

Player::Player()
    : mRenderVidFrameObserver(nullptr)
    , mDecoder(nullptr)
{
}

Player::~Player()
{
}

void Player::setRenderObserver(RenderVidFrameObserver* obs)
{
    mRenderVidFrameObserver = obs;
}

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    auto cleanupVideo = [](FrameQueue::VideoFrame* inFrame){
        if (inFrame->second)
        {
            FrameQueue::release(inFrame->first, inFrame->second);
        }
    };

    if (!frame)
    {
        return;
    }

    if (!mRenderVidFrameObserver)
    {
        cleanupVideo(frame);
        return;
    }
    auto [xRes, yRes] = mRenderVidFrameObserver->getOutDim();
    // render the frame
    if (xRes && yRes)
    {
        auto x = xRes;
        auto y = yRes;

        std::visit(FrameQueue::overloaded {
            [this, x, y](FrameQueue::VideoFrameStr& arg)
            {
                size_t size = 0;
                auto scaledFrame = convScaleFrame(arg, x, y, size);
                mRenderVidFrameObserver->onRender(std::move(scaledFrame), size);
            },
            [this, cleanupCb = frame->second](FrameQueue::VideoFrameCompressedStr& arg)
            {
                mDecoder->pushToDecode(arg, cleanupCb);
            }
        }, frame->first);
    }
    cleanupVideo(frame);
}

void Player::onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

#if 0
    //TODO: play audio
#else
//    LOGW("dump audio, remaining:%d\n", remainingCount);
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

