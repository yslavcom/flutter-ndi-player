/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:06:46  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:06:46 . All rights reserved */

#pragma once

#include "texture.hpp"

#include "interfaces/frame_observer.hpp"
#include "common/frame-queue.hpp"
#include "common/conv-scale.hpp"

#include <memory>
#include <mutex>

#define USE_EXTERN_TEXTURE (1)

//class EglWrap;

class Player: public VideoFrameObserver, public AudioFrameObserver
{
public:
    Player();
    ~Player();

    void setRenderObserver(RenderVidFrameObserver* obs);

    void init(void* surfaceTexture);
    struct Dimensions
    {
        Dimensions()
        {}

        Dimensions(unsigned w, unsigned h)
            : xRes(w), yRes(h)
        {}

        Dimensions(const Dimensions& rhs)
        {
            xRes = rhs.xRes;
            yRes = rhs.yRes;
        }


        unsigned xRes;
        unsigned yRes;
    };

    void setTexDimensions(unsigned hor, unsigned ver);
    void setViewportDimensions(unsigned hor, unsigned ver);

    bool loadTex(uint8_t* frameBuf);

private:
    void onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount) override ;
    void onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount) override ;

    std::unique_ptr<uint8_t[]> convScaleFrame(const FrameQueue::VideoFrameStr& frame, size_t& size);
    std::unique_ptr<ConvertScale> mConvertScale;

    Dimensions mDimViewport;
    Dimensions mDimTex;

    std::mutex mMu;

    void renderFrame(FrameQueue::VideoFrameStr& frame);

#if !USE_EXTERN_TEXTURE
    std::unique_ptr<Texture2D> mTexture2D;
#endif
//    EglWrap *mEglWrap;

    RenderVidFrameObserver* mRenderVidFrameObserver;
};
