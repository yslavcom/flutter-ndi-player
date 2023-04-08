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

void RxFrameController::run()
{
    processVideoQueue();
    processAudioQueue();
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
    }
}

void RxFrameController::processAudioQueue()
{
    auto& queue = mAudioRxQueue;
    if (queue.getCount())
    {
        if (queue.getCount())
        {
            FrameQueue::AudioFrame frame;
            if (queue.read(frame))
            {
#if 0
                for (auto& el: mAudioFrameObservers)
                {
                    el->onFrame(&frame, queue.getCount());
                }
#else
                frame.second(frame.first.opaque);
                LOGW("Dump audio frame\n");
#endif
            }
        }
    }
}