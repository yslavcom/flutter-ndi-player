#include "decoder-loop.hpp"
#include "common/logger.hpp"

#include <chrono>

#define _DBG_DECLOOP

#ifdef _DBG_DECLOOP
    #define DBG_DECLOOP(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_DECLOOP(format, ...)
#endif

DecoderLoop::DecoderLoop(Video::Decoder* decoder, std::mutex& decMu, FrameQueue::VideoRx* vidFramesToDecode, FrameQueue::VideoRx* decodedVideoFrames)
    : mVideoDecoder(decoder)
    , mVidFramesToDecode(vidFramesToDecode)
    , mDecodedVideoFrames(decodedVideoFrames)
    , mTerminateProcessFrames{false}
    , mDecMu(decMu)
{
    DBG_DECLOOP("DecoderLoop:%p, %p, %p\n", mVideoDecoder, mVidFramesToDecode, mDecodedVideoFrames);
}

DecoderLoop::~DecoderLoop()
{
    DBG_DECLOOP("~DecoderLoop START\n");
    mTerminateProcessFrames = true;
    auto stats = mProcessFramesRes.get();
    LOGW("stats, frames decoded:%d\n", stats.framesDecoded);
    if (mDecodedVideoFrames)
    {
        mDecodedVideoFrames->flush();
    }
    mDiagnostics.get();
    DBG_DECLOOP("~DecoderLoop END\n");
}

bool DecoderLoop::run()
{
    DBG_DECLOOP("DecoderLoop::run\n");
    if (!mVideoDecoder || !mVidFramesToDecode)
    {
        return false;
    }
    mProcessFramesRes = std::async(std::launch::async, &DecoderLoop::processFrames, this);
    mDiagnostics = std::async(std::launch::async, &DecoderLoop::diagnostics, this);
    return true;
}

DecoderLoop::Statistics DecoderLoop::processFrames()
{
    Statistics stats{};

    while(!mTerminateProcessFrames)
    {
        if (!mVideoDecoder->isStarted())
        {
            mVideoDecoder->start();
            break;
        }
    }

    while(!mTerminateProcessFrames)
    {
        if (mVidFramesToDecode->getCount())
        {
            FrameQueue::VideoFrame frame;
            mVidFramesToDecode->read(frame);
            auto& compressedFrame = std::get<FrameQueue::VideoFrameCompressedStr>(frame.first);
            auto buf = compressedFrame.p_data;

            if (!mTerminateProcessFrames)
            {
        //        mVideoDecoder->setSpsPps(compressedFrame.sps, compressedFrame.pps);

                // keep pushing the rame while decoder is rerady to accept it
                while(!mTerminateProcessFrames && !mVideoDecoder->enqueueFrame(compressedFrame.p_data, compressedFrame.dataSizeBytes));
                stats.framesDecoded ++;
            }
        }

        if (!mTerminateProcessFrames)
        {
            mVideoDecoder->retrieveFrame();
        }
    }

    return stats;
}

void DecoderLoop::diagnostics()
{
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        mVideoDecoder->diagnostics(mVideoDecoder);

        if (mTerminateProcessFrames)
        {
            return;
        }
    }
}
