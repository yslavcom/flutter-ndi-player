#include "player.hpp"

#include "common/logger.hpp"

#include <android_native_app_glue.h>

#include <GLES/gl.h>
#include <EGL/egl.h>

class EglWrap
{
public:
    EglWrap()
        : mEglShareContext(EGL_NO_CONTEXT)
    {}

    bool init(EGLNativeWindowType texture)
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

        eglBindAPI(EGL_OPENGL_ES_API);

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


        mEglTexture = texture;
        mEglSurface = eglCreateWindowSurface(mEglDisplay, mEGLConfig, mEglTexture, NULL);
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

    ~EglWrap()
    {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(mEglDisplay, mEglSurface);
        eglDestroyContext(mEglDisplay, mEglContext);
        eglTerminate(mEglDisplay);
    }

private:
    EGLDisplay mEglDisplay;
    EGLint mEglMajor;
    EGLint mEglMinor;
    EGLConfig mEGLConfig;
    EGLContext mEglShareContext;
    EGLContext mEglContext;
    EGLSurface mEglSurface;
    EGLNativeWindowType mEglTexture;
#if 1
    EGLConfig chooseEglConfig() {
        EGLConfig configs;
        EGLint numConfigs;

        if (!eglChooseConfig(mEglDisplay, mEglConfigSpecs, &configs, 1, &numConfigs))
        {
            LOGE("EGL choose config failed: %s\n", getEGLErrorString(eglGetError()));
            return nullptr;
        }
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

namespace
{
    std::unique_ptr<EglWrap> mEglWrap;
}

Player::Player()
    : mEglWrap(nullptr)
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
    if (!::mEglWrap)
    {
        ::mEglWrap = std::make_unique<EglWrap>();
        mEglWrap = ::mEglWrap.get();
#if 1
        //mEglWrap->init((EGLNativeWindowType)mTexture2D->handle());
        mEglWrap->init(0);
#endif
    }
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
    //TODO: play audio
#else
//    LOGW("dump audio, remaining:%d\n", remainingCount);
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
