/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 16:46:09  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 16:46:09 . All rights reserved */

#include "render.hpp"

bool RenderVid::init(NativeWindowType win)
{
    if (initEgl(win))
    {
        return initTex();
    }
    return false;
}

bool RenderVid::clear()
{
    mEglWrap = nullptr;
    mTexture2D = nullptr;

    return true;
}

bool RenderVid::initEgl(NativeWindowType win)
{
    mEglWrap.reset(new EglWrap(win));
    return mEglWrap->init();
}

bool RenderVid::initTex()
{
#if 0
    mTexture2D.reset(new Texture2D());
    mTexture2D->init();
#endif
    return true;
}

void RenderVid::render(uint8_t* data, unsigned xRes, unsigned yRes)
{
//    mEglWrap->makeCurrent();
    mEglWrap->clearScreen();
#if 0
    mTexture2D->bind();
    mTexture2D->loadImage(0, GL_RGBA, xRes, yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    mTexture2D->unbind();
#endif
    mEglWrap->swapBuffers();
}
