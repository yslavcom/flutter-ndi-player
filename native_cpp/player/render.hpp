/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 07:21:12  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 07:21:12 . All rights reserved */

#pragma once

#include "egl_wrap.hpp"
#include "texture.hpp"

#include <memory>

class RenderVid
{
public:
    RenderVid(unsigned xRes, unsigned yRes)
    {
        glViewport(0, 0, xRes, yRes);
    }

    void render(uint8_t* data, unsigned xRes, unsigned yRes);

    bool init(NativeWindowType win);
    bool clear();

private:
    bool initEgl(NativeWindowType win);
    bool initTex();

    std::unique_ptr<EglWrap> mEglWrap;
    std::unique_ptr<Texture2D> mTexture2D;
};
