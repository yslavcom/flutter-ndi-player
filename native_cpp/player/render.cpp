/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 16:46:09  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 16:46:09 . All rights reserved */

#include "render.hpp"

RenderVid::RenderVid(unsigned xRes, unsigned yRes)
    : mXres(xRes)
    , mYres(yRes)
{}

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
    mTexture2D.reset(new Texture2D());
    mTexture2D->init();
    return mTexture2D->bind();
}

void RenderVid::render(uint8_t* data)
{
}
