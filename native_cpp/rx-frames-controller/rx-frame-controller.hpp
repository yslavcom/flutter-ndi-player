#pragma once

#include "common/frame-queue.hpp"
#include "interfaces/frame_observer.hpp"

#include <set>
#include <mutex>

class RxFrameController
{
public:
    RxFrameController(FrameQueue::VideoRx& videoRxQueue, FrameQueue::AudioRx& audioRxQueue)
        : mVideoRxQueue(videoRxQueue)
        , mAudioRxQueue(audioRxQueue)
        , mVidFramesDecoded(nullptr)
    {}

    void installVideoFrameObs(VideoFrameObserver* obs);
    void uninstallVideoFrameObs(VideoFrameObserver* obs);

    void installAudioFrameObs(AudioFrameObserver* obs);
    void uninstallAudioFrameObs(AudioFrameObserver* obs);

    void run();

    void setDecodedFramesQueue(FrameQueue::VideoRx* decodedFramesQueue);
private:
    // Video queue where video frames are pushed from RX.
    // Can be both decompressed (ready for render) and compressed (subject to decompression by a decoder)
    FrameQueue::VideoRx& mVideoRxQueue;
    FrameQueue::AudioRx& mAudioRxQueue;

    // Video queue where video frames are pushed from decodr after decompression and they are ready for render
    FrameQueue::VideoRx* mVidFramesDecoded;
    std::mutex mDecodedQueueInstallMu;

    std::set<VideoFrameObserver*> mVideoFrameObservers;
    std::set<AudioFrameObserver*> mAudioFrameObservers;

    void processVideoQueue();
    void processDecodedVideoQueue();
    void processAudioQueue();
};