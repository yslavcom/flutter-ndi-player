/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 06:53:53  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 06:53:53 . All rights reserved */

#include "player.hpp"
#include "common/logger.hpp"

#include <android_native_app_glue.h>

Player::Player()
    : mRenderVidFrameObserver(nullptr)
{
//    glViewport(0, 0, mDimViewport.xRes, mDimViewport.yRes);
}

Player::~Player()
{
#if !USE_EXTERN_TEXTURE
    mTexture2D.reset();
#endif
}

void Player::setRenderObserver(RenderVidFrameObserver* obs)
{
    mRenderVidFrameObserver = obs;
}

bool Player::loadTex(uint8_t* frameBuf)
{
    bool ret = true;
#if !USE_EXTERN_TEXTURE
    mTexture2D->bind();
    ret = mTexture2D->loadImage(0, GL_RGBA, mDimTex.xRes, mDimTex.yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameBuf);
    mTexture2D->unbind();
#endif
    return ret;
}

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    auto cleanupVideo = [](auto frame){
        if (frame->second)
        {
            frame->second(frame->first.opaque);
        }
    };

    if (!frame)
    {
        return;
    }

    if (!mRenderVidFrameObserver)
    {
        cleanupVideo(frame);
        return;
    }
    auto [xRes, yRes] = mRenderVidFrameObserver->getOutDim();
    // render the frame
    if (xRes && yRes)
    {
        size_t size;
        auto scaledFrame = convScaleFrame(frame->first, xRes, yRes, size);
        mRenderVidFrameObserver->onRender(std::move(scaledFrame), size);
    }
    cleanupVideo(frame);
}

void Player::onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

#if 0
    //TODO: play audio
#else
//    LOGW("dump audio, remaining:%d\n", remainingCount);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
    }
#endif
}

std::unique_ptr<uint8_t[]> Player::convScaleFrame(const FrameQueue::VideoFrameStr& frame, unsigned xRes, unsigned yRes, size_t& size)
{
    const int outputPixelComponentscount = 4; //RGBA

    size = xRes * yRes * outputPixelComponentscount; // RGBA
    auto rgbaFrame = std::make_unique<uint8_t[]>(size);
    if (!rgbaFrame)
    {
        LOGE("Failed to allocate memory for rgba frame\n");
        return nullptr;
    }

    if (!mConvertScale)
    {
        mConvertScale = std::make_unique<ConvertScale>(frame.xres, frame.yres, xRes, yRes);
    }

    uint8_t* src[3] = {frame.data, nullptr, nullptr};
    int bytes_in_line = (0 == frame.stride) ? frame.xres : frame.stride;
    int srcLineSize[4] = {bytes_in_line, bytes_in_line, bytes_in_line, bytes_in_line};

    uint8_t* dest[4] = {rgbaFrame.get(), nullptr, nullptr, nullptr};
    auto lineSize = static_cast<int>(xRes) * outputPixelComponentscount;
    int destLineSize[4] = {lineSize, lineSize, lineSize, lineSize};

    if (0 < mConvertScale->scale(xRes, yRes, frame.xres, frame.yres, src, srcLineSize, dest, destLineSize))
    {
        return rgbaFrame;
    }
    LOGE("scale faild, size:%d, xRes:%d, yRes:%d, frame.xres:%d, frame.yres:%d, frame.stride:%d, bytes_in_line:%d\n",
        size, xRes, yRes, frame.xres, frame.yres, frame.stride, bytes_in_line);
    return nullptr;
}

void Player::renderFrame(FrameQueue::VideoFrameStr& frame)
{
    size_t size = 0;
//    auto framePtr = convScaleFrame(frame, size);
    std::unique_ptr<uint8_t[]> framePtr;
    if (framePtr)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up orthographic projection
#if 0
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof(0, mDimTex.xRes, mDimTex.yRes, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
#endif
        // load texture
        loadTex(framePtr.get());
#if 0
        // render to quad

        // enable 2d texture capability,
        //If enabled and no fragment shader is active, two-dimensional texturing is performed
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS); // render rectangle
            glTexCoord2d(0, 0); // set the current texture coord
            glVertex2i(0, 0); // vertex coordinate

            glTexCoord2d(1, 0);
            glVertex2i(mDimTex.xRes, 0);

            glTexCoord2d(1, 1);
            glVertex2i(mDimTex.xRes, mDimTex.yRes);

            glTexCoord2d(0, 1);
            glVertex2i(0, mDimTex.yRes);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(mWindow);

        glFlush();
#endif
    }
}
