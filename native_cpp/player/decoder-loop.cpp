#include "decoder-loop.hpp"
#include "common/logger.hpp"

#include <chrono>
#include <thread>

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
    , mStart(true)
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

void DecoderLoop::init()
{
    mStart = true;
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
        auto cleanupFrame = [](FrameQueue::ReleaseCb cb, void* opaque){
            if (cb)
            {
                cb(opaque);
            }
        };

        if (mVidFramesToDecode->getCount())
        {
            FrameQueue::VideoFrame frame;
            mVidFramesToDecode->read(frame);
            auto& compressedFrame = std::get<FrameQueue::VideoFrameCompressedStr>(frame.first);

            std::vector<uint8_t> sps;
            std::vector<uint8_t> pps;
            // We should strip off the prepended header (4 reserved to the header).
            auto si = H26x::tryParseServiceInfo(H26x::FourCC(compressedFrame.fourCC).getType(), compressedFrame.p_data+4, compressedFrame.dataSizeBytes-4, compressedFrame.hdrSize-4);
            if (si)
            {
                if (si->sps.size())
                {
                    sps = std::move(si->sps);
                }
                if (si->pps.size())
                {
                    pps = std::move(si->pps);
                }
            }

            bool isContinue = true;
            if (mStart)
            {
                isContinue = false;
                if (si)
                {
                    isContinue = si->isKeyFrame;
                    if (isContinue)
                    {
                        mStart = false;
                    }
                }
            }

            if (!isContinue)
            {
                cleanupFrame(frame.second, compressedFrame.opaque);
                continue;
            }

            // We should strip off the prepended header, again.
            compressedFrame.p_data = compressedFrame.p_data + compressedFrame.hdrSize + sps.size() + pps.size();
            compressedFrame.dataSizeBytes = compressedFrame.dataSizeBytes - compressedFrame.hdrSize - (sps.size() + pps.size());

            if (!mTerminateProcessFrames)
            {
//                mVideoDecoder->setSpsPps(sps, pps);

                // keep pushing the frame while decoder is ready to accept it
                while(!mTerminateProcessFrames && !mVideoDecoder->enqueueFrame(compressedFrame.p_data, compressedFrame.dataSizeBytes));

                cleanupFrame(frame.second, compressedFrame.opaque);

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
