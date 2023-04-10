#include "player.hpp"

#include "common/logger.hpp"

Player::Player()
{}

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

    // render the frame
    size_t size;
    auto scaledFrame = convScaleFrame(frame->first, size);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
    }
}

void Player::onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

#if 0
    // play audio
#else
    LOGW("dump audio, remaining:%d\n", remainingCount);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
    }
#endif
}

void Player::setTexDimensions(unsigned hor, unsigned ver)
{
    std::lock_guard lk(mMu);

    mDimTex = {hor, ver};
}

std::unique_ptr<uint8_t[]> Player::convScaleFrame(const FrameQueue::VideoFrameStr& frame, size_t& size)
{
    Dimensions dimTex;

    {
        std::lock_guard lk(mMu);
        dimTex = mDimTex;
    }

    const int outputPixelComponentscount = 4; //RGBA

    auto xRes = dimTex.xRes;
    auto yRes = dimTex.yRes;

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

void Player::renderFrame(FrameQueue::VideoFrameStr& frame)
{
    size_t size = 0;
    auto framePtr = convScaleFrame(frame, size);
    if (framePtr)
    {
    }
}
