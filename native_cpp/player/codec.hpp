#pragma once

#include "common/frame-queue.hpp"
#include "common/logger.hpp"

#include "sps_pps_parser.hpp"

#include <stdint.h>
#include <vector>

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
        , mFourCC(H26x::FourCC::Undefined())
        , mXres(0)
        , mYres(0)
    {}

    virtual ~Decoder(){}

    virtual bool create(uint32_t fourcc) = 0;
    virtual bool configure() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual void release() = 0;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) = 0;
    virtual bool isReady() const = 0;
    virtual bool isStarted() const = 0;
    virtual void requestSetup() = 0;

    virtual bool retrieveFrame() = 0;

    virtual void init(void* nativeWindow) = 0;
    virtual void diagnostics(void* userData) = 0;

    void setVidFramesToDecode(FrameQueue::VideoRx* vidFramesToDecode)
    {
        mVidFramesToDecode = vidFramesToDecode;
    }

    bool pushToDecode(FrameQueue::VideoFrameCompressedStr& frame, FrameQueue::ReleaseCb releaseCb)
    {
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

    void setCodecFourCC(H26x::FourCC fourcc)
    {
        mFourCC = fourcc;
    }

    uint32_t getCodecFourCC() const
    {
        return mFourCC.word;
    }

    void setDimensions(unsigned xRes, unsigned yRes)
    {
        mXres = xRes;
        mYres = yRes;
    }

    virtual void setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps) = 0;

protected:
    FrameQueue::VideoRx* mVidFramesToDecode;
    FrameQueue::VideoRx* mDecodedVideoFrames;

    H26x::FourCC mFourCC;

    unsigned mXres;
    unsigned mYres;
};
} // namespace Video
