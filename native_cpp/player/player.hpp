/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:06:46  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:06:46 . All rights reserved */

#pragma once

#include "texture.hpp"

#include "interfaces/frame_observer.hpp"
#include "common/frame-queue.hpp"
#include "common/conv-scale.hpp"

#include <memory>

#define USE_EXTERN_TEXTURE (1)

//class EglWrap;

class Player: public VideoFrameObserver, public AudioFrameObserver
{
public:
    Player();
    ~Player();

    void setRenderObserver(RenderVidFrameObserver* obs);

    bool loadTex(uint8_t* frameBuf);

private:
    void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) override ;
    void onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount) override ;

    std::unique_ptr<uint8_t[]> convScaleFrame(const FrameQueue::VideoFrameStr& frame, unsigned xRes, unsigned yRes, size_t& size);
    std::unique_ptr<ConvertScale> mConvertScale;

    void renderFrame(FrameQueue::VideoFrameStr& frame);

#if !USE_EXTERN_TEXTURE
    std::unique_ptr<Texture2D> mTexture2D;
#endif
//    EglWrap *mEglWrap;

    RenderVidFrameObserver* mRenderVidFrameObserver;
};
