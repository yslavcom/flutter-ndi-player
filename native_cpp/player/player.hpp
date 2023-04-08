#pragma once

#include "interfaces/frame_observer.hpp"
#include "common/frame-queue.hpp"

class Player: public VideoFrameObserver, public AudioFrameObserver
{
public:

private:
    void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) override ;
    void onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount) override ;
};
