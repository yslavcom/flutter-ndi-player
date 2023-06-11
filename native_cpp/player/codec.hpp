
#pragma once

#include "common/frame-queue.hpp"
#include "common/logger.hpp"

#include <stdint.h>
namespace Video
{
#define _DBG_DECODER
#ifdef _DBG_DECODER
    #define DBG_DECODER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_DECODER(format, ...)
#endif
class Decoder
{
public:
    Decoder()
        : mVidFramesToDecode(nullptr)
        , mDecodedVideoFrames(nullptr)
    {}

    virtual ~Decoder(){}

    virtual bool create() = 0;
    virtual bool configure() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) = 0;
    virtual bool isReady() const = 0;
    virtual void requestSetup() = 0;

    virtual bool retrieveFrame() = 0;

    virtual void init(unsigned xRes, unsigned yRes, void* nativeWindow) = 0;

    void setVidFramesToDecode(FrameQueue::VideoRx* vidFramesToDecode)
    {
        mVidFramesToDecode = vidFramesToDecode;
    }

    bool pushToDecode(FrameQueue::VideoFrameCompressedStr& frame, FrameQueue::ReleaseCb releaseCb)
    {
        DBG_DECODER("pushToDecode:%p (%p, %p)\n", mVidFramesToDecode, &frame, &releaseCb);
        if (mVidFramesToDecode)
        {
            return mVidFramesToDecode->push(std::make_pair(frame, releaseCb));
        }
        return false;
    }

    void setDecodedFramesQueue(FrameQueue::VideoRx* decodedVideoFrames)
    {
        mDecodedVideoFrames = decodedVideoFrames;
    }

protected:
    FrameQueue::VideoRx* mVidFramesToDecode;
    FrameQueue::VideoRx* mDecodedVideoFrames;
};
} // namespace Video
