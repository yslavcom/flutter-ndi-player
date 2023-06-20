#pragma once

#include "common/frame-queue.hpp"
#include "interfaces/frame_observer.hpp"
#include "interfaces/input-control.hpp"

#include <set>
#include <mutex>
#include <chrono>

class RxFrameController
{
public:
    RxFrameController(FrameQueue::VideoRx& videoRxQueue, FrameQueue::AudioRx& audioRxQueue, InputControl* inputControl)
        : mVideoRxQueue(videoRxQueue)
        , mAudioRxQueue(audioRxQueue)
        , mTimeRefr{std::chrono::steady_clock::now()}
        , mFrameCount(0)
        , mInputControl(inputControl)
    {}

    void installVideoFrameObs(VideoFrameObserver* obs);
    void uninstallVideoFrameObs(VideoFrameObserver* obs);

    void installAudioFrameObs(AudioFrameObserver* obs);
    void uninstallAudioFrameObs(AudioFrameObserver* obs);

    void run();

private:
    // Video queue where video frames are pushed from RX.
    // Can be both decompressed (ready for render) and compressed (subject to decompression by a decoder)
    FrameQueue::VideoRx& mVideoRxQueue;
    FrameQueue::AudioRx& mAudioRxQueue;

    std::set<VideoFrameObserver*> mVideoFrameObservers;
    std::set<AudioFrameObserver*> mAudioFrameObservers;

    void processVideoQueue();
    void processAudioQueue();
    void checkFramesCountInTime();

    std::chrono::steady_clock::time_point mTimeRefr;
    unsigned mFrameCount;

    InputControl* mInputControl;
};