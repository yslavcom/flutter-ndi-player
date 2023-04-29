/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-04-29 05:59:14  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-04-29 05:59:14 . All rights reserved */
#pragma once

#include "common/logger.hpp"

#include <GLES/gl.h>
#include <EGL/egl.h>

class EglWrap
{
public:
    EglWrap(EGLNativeWindowType window)
        : mEglShareContext(EGL_NO_CONTEXT)
        , mEGLNativeWindowType(window)
    {}

    ~EglWrap()
    {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(mEglDisplay, mEglSurface);
        eglDestroyContext(mEglDisplay, mEglContext);
        eglTerminate(mEglDisplay);
    }

    bool init()
    {
        mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (mEglDisplay == EGL_NO_DISPLAY)
        {
            LOGE("can't load EGL mEglDisplay: %s\n", getEGLErrorString(eglGetError()));
            return false;
        }

        if (!eglInitialize(mEglDisplay, &mEglMajor, &mEglMinor))
        {
            LOGE("EGL initialize failed: %s\n", getEGLErrorString(eglGetError()));
        }

//        eglBindAPI(EGL_OPENGL_ES_API);

        mEGLConfig = chooseEglConfig();

        EGLint attributes[] =
        {
            EGL_CONTEXT_CLIENT_VERSION,
            2,
            EGL_NONE
        };

        mEglContext = eglCreateContext(mEglDisplay, mEGLConfig, mEglShareContext, attributes);
        if (mEglContext == EGL_NO_CONTEXT)
        {
            LOGE("EGL create mEglContext failed: %s\n", getEGLErrorString(eglGetError()));
            return false;
        }
        {
            LOGW("mEglContext:%p\n", mEglContext);
        }


        mEglSurface = eglCreateWindowSurface(mEglDisplay, mEGLConfig, mEGLNativeWindowType, NULL);
        if (mEglSurface == nullptr || mEglSurface == EGL_NO_SURFACE)
        {
            LOGE("GL Error: %s\n", getEGLErrorString(eglGetError()));
        }

        if (!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
        {
            LOGE("EGL make current failed: %s\n", getEGLErrorString(eglGetError()));
        }

        LOGW("Init ok\n");
        return true;
    }

    void clearScreen()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void swapBuffers()
    {
        if (!eglSwapBuffers(mEglDisplay, mEglSurface))
        {
            LOGE("SwapBuffer fail: %s\n", getEGLErrorString(eglGetError()));
        }
    }

private:
    EGLDisplay mEglDisplay;
    EGLint mEglMajor;
    EGLint mEglMinor;
    EGLConfig mEGLConfig;
    EGLContext mEglShareContext;
    EGLContext mEglContext;
    EGLSurface mEglSurface;
    EGLNativeWindowType mEGLNativeWindowType;

#if 1
    EGLConfig chooseEglConfig() {
        EGLConfig configs;
        EGLint numConfigs;

        if (!eglChooseConfig(mEglDisplay, mEglConfigSpecs, &configs, 1, &numConfigs))
        {
            LOGE("EGL choose config failed: %s\n", getEGLErrorString(eglGetError()));
            return nullptr;
        }
        LOGW("numConfigs:%d\n", numConfigs);
        return configs;
    }

    static constexpr EGLint mEglConfigSpecs[] = {
        EGL_RENDERABLE_TYPE, 4,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_SAMPLES, 4,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };
#endif

    #define CASE_STR( value ) case value: return #value;
    const char* getEGLErrorString( EGLint error )
    {
        switch( error )
        {
        CASE_STR( EGL_SUCCESS             )
        CASE_STR( EGL_NOT_INITIALIZED     )
        CASE_STR( EGL_BAD_ACCESS          )
        CASE_STR( EGL_BAD_ALLOC           )
        CASE_STR( EGL_BAD_ATTRIBUTE       )
        CASE_STR( EGL_BAD_CONTEXT         )
        CASE_STR( EGL_BAD_CONFIG          )
        CASE_STR( EGL_BAD_CURRENT_SURFACE )
        CASE_STR( EGL_BAD_DISPLAY         )
        CASE_STR( EGL_BAD_SURFACE         )
        CASE_STR( EGL_BAD_MATCH           )
        CASE_STR( EGL_BAD_PARAMETER       )
        CASE_STR( EGL_BAD_NATIVE_PIXMAP   )
        CASE_STR( EGL_BAD_NATIVE_WINDOW   )
        CASE_STR( EGL_CONTEXT_LOST        )
        default: return "Unknown";
        }
    }
    #undef CASE_STR
};
