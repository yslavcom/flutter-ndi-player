#include "decoder-loop.hpp"
#include "common/logger.hpp"

DecoderLoop::DecoderLoop(Video::Decoder* decoder, FrameQueue::VideoRx* vidFramesToDecode, FrameQueue::VideoRx* decodedVideoFrames)
    : mVideoDecoder(decoder)
    , mVidFramesToDecode(vidFramesToDecode)
    , mDecodedVideoFrames(decodedVideoFrames)
{
}

DecoderLoop::~DecoderLoop()
{
    mTerminateProcessFrames = true;
    mProcessFramesRes.get();
}

bool DecoderLoop::run()
{
    LOGW("DecoderLoop::run\n");
    if (!mVideoDecoder || !mVidFramesToDecode)
    {
        return false;
    }
    mProcessFramesRes = std::async(std::launch::async, &DecoderLoop::processFrames, this);
    return true;
}

DecoderLoop::Statistics DecoderLoop::processFrames()
{
    Statistics stats{};

    for (;;)
    {
        LOGW("getCount:%d\n", mVidFramesToDecode->getCount());
        if (mVidFramesToDecode->getCount())
        {
            FrameQueue::VideoFrame frame;
            mVidFramesToDecode->read(frame);

            mVideoDecoder->start();
        }
    }

    return stats;
}