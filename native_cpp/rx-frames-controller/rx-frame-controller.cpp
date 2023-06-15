#include "rx-frame-controller.hpp"
#include "common/logger.hpp"

void RxFrameController::installVideoFrameObs(VideoFrameObserver* obs)
{
    if (!obs)
    {
        return;
    }
    mVideoFrameObservers.emplace(obs);
}

void RxFrameController::uninstallVideoFrameObs(VideoFrameObserver* obs)
{
    if (!obs)
    {
        return;
    }
    mVideoFrameObservers.erase(obs);
}

void RxFrameController::installAudioFrameObs(AudioFrameObserver* obs)
{
    if (!obs)
    {
        return;
    }
    mAudioFrameObservers.emplace(obs);
}

void RxFrameController::uninstallAudioFrameObs(AudioFrameObserver* obs)
{
    if (!obs)
    {
        return;
    }
    mAudioFrameObservers.erase(obs);
}

void RxFrameController::run()
{
    processVideoQueue();
    processDecodedVideoQueue();

    processAudioQueue();
}

void RxFrameController::setDecodedFramesQueue(FrameQueue::VideoRx* decodedFramesQueue)
{
    std::lock_guard lk(mDecodedQueueInstallMu);
    mVidFramesDecoded = decodedFramesQueue;
}

void RxFrameController::processVideoQueue()
{
    auto& queue = mVideoRxQueue;
    if (queue.getCount())
    {
        FrameQueue::VideoFrame frame;
        if (queue.read(frame))
        {
            for (auto& el: mVideoFrameObservers)
            {
                el->onFrame(&frame, queue.getCount());
            }
        }

        // It must be cleaned after all observers had a chance to process the frame
        if (frame.second)
        {
            FrameQueue::release(frame.first, frame.second);
        }
    }
}

void RxFrameController::processDecodedVideoQueue()
{
    std::lock_guard lk(mDecodedQueueInstallMu);
    if (!mVidFramesDecoded)
    {
        return;
    }
    auto& queue = *mVidFramesDecoded;
    if (queue.getCount())
    {
        FrameQueue::VideoFrame frame;
        if (queue.read(frame))
        {
            for (auto& el: mVideoFrameObservers)
            {
                el->onFrame(&frame, queue.getCount());
            }
        }
    }
}

void RxFrameController::processAudioQueue()
{
    auto& queue = mAudioRxQueue;
    if (queue.getCount())
    {
        FrameQueue::AudioFrame frame;
        if (queue.read(frame))
        {
            for (auto& el: mAudioFrameObservers)
            {
                el->onFrame(&frame, queue.getCount());
            }
        }
    }
}