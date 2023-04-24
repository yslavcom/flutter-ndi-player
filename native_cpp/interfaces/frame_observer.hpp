#pragma once

#include "common/frame-queue.hpp"

#include <memory>

class VideoFrameObserver
{
public:
    virtual void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) = 0;
};

class AudioFrameObserver
{
public:
    virtual void onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount) = 0;
};

class RenderVidFrameObserver
{
public:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) = 0;
};
