/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:06:46  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:06:46 . All rights reserved */

#pragma once

#include "texture.hpp"
#include "codec.hpp"

#include "interfaces/frame_observer.hpp"
#include "common/frame-queue.hpp"
#include "common/conv-scale.hpp"

#include <memory>

class Player: public VideoFrameObserver, public AudioFrameObserver
{
public:
    Player();
    virtual ~Player();

    void setRenderObserver(RenderVidFrameObserver* obs);
    void setDecoder(Video::Decoder* decoder);
    void reStart();

private:
    void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) override ;
    void onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount) override ;

    std::unique_ptr<uint8_t[]> convScaleFrame(const FrameQueue::VideoFrameStr& frame, unsigned xRes, unsigned yRes, size_t& size);
    std::unique_ptr<ConvertScale> mConvertScale;

    RenderVidFrameObserver* mRenderVidFrameObserver;
    Video::Decoder* mVideoDecoder;
    std::mutex mDecoderMu;

    bool mAudioInitialised;

    enum class State
    {
        Idle = 0,
        Connecting,
        Connected
    };
    State mState;
};
