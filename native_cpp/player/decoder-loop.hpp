#pragma once

#include "codec.hpp"
#include "common/frame-queue.hpp"

#include <future>
#include <mutex>

class DecoderLoop
{
public:
    DecoderLoop(Video::Decoder* decoder, std::mutex& decMu, FrameQueue::VideoRx* vidFramesToDecode, FrameQueue::VideoRx* decodedVideoFrames);
    ~DecoderLoop();
    bool run();

private:
    Video::Decoder* mVideoDecoder;
    FrameQueue::VideoRx* mVidFramesToDecode;
    FrameQueue::VideoRx* mDecodedVideoFrames;

    struct Statistics
    {
        Statistics() : framesDecoded(0)
        {}
        unsigned framesDecoded;
    };

    Statistics processFrames();
    std::future<Statistics> mProcessFramesRes;

    void diagnostics();
    std::future<void> mDiagnostics;

    std::atomic<bool> mTerminateProcessFrames;

    std::mutex& mDecMu;
};