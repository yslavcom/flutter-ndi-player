#include "player.hpp"

#include "common/logger.hpp"

Player::Player()
{
    glViewport(0, 0, mDimViewport.xRes, mDimViewport.yRes);
}

Player::~Player()
{
    mTexture2D.reset();
}

void Player::init()
{
    mTexture2D = std::make_unique<Texture2D>();
}

bool Player::loadTex(uint8_t* frameBuf)
{
    mTexture2D->bind();
    auto ret = mTexture2D->loadImage(0, GL_RGBA, mDimTex.xRes, mDimTex.yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameBuf);
    mTexture2D->unbind();
    return ret;
}

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

    // render the frame
    size_t size;
    auto scaledFrame = convScaleFrame(frame->first, size);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
    }
}

void Player::onFrame(FrameQueue::AudioFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

#if 0
    // play audio
#else
    LOGW("dump audio, remaining:%d\n", remainingCount);
    if (frame->second)
    {
        frame->second(frame->first.opaque);
    }
#endif
}

void Player::setTexDimensions(unsigned hor, unsigned ver)
{
    std::lock_guard lk(mMu);

    mDimTex = {hor, ver};
}

void Player::setViewportDimensions(unsigned hor, unsigned ver)
{
    std::lock_guard lk(mMu);

    mDimViewport = {hor, ver};
}

std::unique_ptr<uint8_t[]> Player::convScaleFrame(const FrameQueue::VideoFrameStr& frame, size_t& size)
{
    Dimensions dimTex;

    {
        std::lock_guard lk(mMu);
        dimTex = mDimTex;
    }

    const int outputPixelComponentscount = 4; //RGBA

    auto xRes = dimTex.xRes;
    auto yRes = dimTex.yRes;

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
    auto framePtr = convScaleFrame(frame, size);
    if (framePtr)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up orthographic projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrthof(0, mDimTex.xRes, mDimTex.yRes, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        // load texture
        loadTex(framePtr.get());

        // render
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
            glTexCoord2d(0, 0);
            glVertex2i(0, 0);

            glTexCoord2d(1, 0);
            glVertex2i(mDimTex.xRes, 0);

            glTexCoord2d(1, 1);
            glVertex2i(mDimTex.xRes, mDimTex.yRes);

            glTexCoord2d(0, 1);
            glVertex2i(0, mDimTex.yRes);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(mWindow);
    }
}
