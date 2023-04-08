#pragma once

#include "common/frame-queue.hpp"
#include "interfaces/frame_observer.hpp"

#include <set>

class RxFrameController
{
public:
    RxFrameController(FrameQueue::VideoRx& videoRxQueue, FrameQueue::AudioRx& audioRxQueue)
        : mVideoRxQueue(videoRxQueue)
        , mAudioRxQueue(audioRxQueue)
    {}

    void installVideoFrameObs(VideoFrameObserver* obs);
    void uninstallVideoFrameObs(VideoFrameObserver* obs);

    void run();
private:
    FrameQueue::VideoRx& mVideoRxQueue;
    FrameQueue::AudioRx& mAudioRxQueue;

    std::set<VideoFrameObserver*> mVideoFrameObservers;

    // TODO: add audio here as well
    // std::set<AudioFrameObserver*> mAudioFrameObservers;

    void processVideoQueue();
    void processAudioQueue();
};