#pragma once

#include "common/frame-queue.hpp"

class VideoFrameObserver
{
public:
    virtual void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) = 0;
};