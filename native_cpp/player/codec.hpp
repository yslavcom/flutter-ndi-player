
#pragma once

#include "common/frame-queue.hpp"

#include <stdint.h>

class Decoder
{
public:
    Decoder(FrameQueue::VideoRx* priorDecoderQueue = nullptr)
        : mPriorDecoderQueue(priorDecoderQueue)
    {}

    virtual ~Decoder(){}

    virtual bool create() = 0;
    virtual bool configure() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) = 0;

    virtual bool retrieveFrame() = 0;

    bool pushToDecode(FrameQueue::VideoFrameCompressedStr& frame, FrameQueue::ReleaseCb releaseCb)
    {
        if (mPriorDecoderQueue)
        {
            return mPriorDecoderQueue->push(std::make_pair(frame, releaseCb));
        }
        return false;
    }

private:
    FrameQueue::VideoRx* mPriorDecoderQueue;
};