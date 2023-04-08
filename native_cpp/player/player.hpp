#pragma once

#include "interfaces/frame_observer.hpp"
#include "common/frame-queue.hpp"

class Player: VideoFrameObserver
{
public:

private:
    void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) override ;
};