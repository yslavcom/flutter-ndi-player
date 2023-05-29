#pragma once

#include "codec.hpp"
#include "common/frame-queue.hpp"

#include <future>

class DecoderLoop
{
public:
    DecoderLoop(Video::Decoder* decoder, FrameQueue::VideoRx* vidFramesToDecode, FrameQueue::VideoRx* decodedVideoFrames);
    ~DecoderLoop();
    bool run();

private:
    Video::Decoder* mDecoder;
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
    std::atomic<bool> mTerminateProcessFrames;
};