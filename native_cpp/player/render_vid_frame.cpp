#include "render_vid_frame.hpp"
#include "common/logger.hpp"

#ifdef ANDROID_PLATFORM
#include "android-codec.hpp"
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#endif // #ifdef ANDROID_PLATFORM

#if LINUX_PLATFORM
#include "linux-decoder.hpp"
#endif

#include <functional>
#include <memory>
#include <cassert>

// #define _DBG_RENDER
#ifdef _DBG_RENDER
    #define DBG_RENDER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_RENDER(format, ...)
#endif

#ifdef ANDROID_PLATFORM
namespace
{
// candidate to be removed
    JavaVM *m_jvm;
    jobject gInterfaceObject;

    const char* pathTex = "com/example/ndi_player/Texture";
    using ReqTextCb_t = std::function<void()>;
    ReqTextCb_t reqTextCb;

    std::mutex mRenderMutex;
    std::unique_ptr<RenderVidFrame> mRenderVidFrame;

    void setReqTextCb(ReqTextCb_t cb)
    {
        reqTextCb = cb;
    }

    bool mIsCompressed = false;
    void requestTexture(void* userData, bool isCompressed)
    {
        LOGW("requestTexture:%p\n", userData);
        // It is important to request the texture form the same thread which will be used for rendering
        mIsCompressed = isCompressed;
        if (reqTextCb)
        {
            reqTextCb();
        }
    }

    ANativeWindow* mWindow = nullptr;

    std::unique_ptr<AndroidDecoder> mVidDecoder;
}

extern "C"
JNIEXPORT jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    m_jvm = vm;

    jclass cls = env->FindClass(pathTex);
    jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
    jobject obj = env->NewObject(cls, constr);
    gInterfaceObject = env->NewGlobalRef(obj);

    // Save the callback function pointer
    setReqTextCb([]() {
        JNIEnv *env;
        JavaVMInitArgs vm_args{};
        vm_args.version = JNI_VERSION_1_6;

        auto ret = m_jvm->AttachCurrentThread(&env, NULL);
        if (ret != JNI_OK || !env)
        {
            LOGE("AttachCurrentThread fail\n");
        }
        else
        {
            jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
            jmethodID method = env->GetStaticMethodID(interfaceClass, "onRequestTex", "()V");

            env->CallStaticVoidMethod(interfaceClass, method);
            m_jvm->DetachCurrentThread();

            LOGW("Requested texture\n");
        }
    });

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_setTextureSize(JNIEnv* env, jobject obj, jint width, jint height)
{
    {
        std::lock_guard lk(mRenderMutex);
        getRenderVidFrame()->setOutDim(width, height);
    }

    LOGW("Tex size, width:%d, height:%d\n", width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_disposeTexture(JNIEnv* env, jobject)
{
    LOGW("Clear surface\n");
    getVideoDecoder()->release();

    {
        std::lock_guard lk(mRenderMutex);
        getRenderVidFrame()->setOutDim(0, 0);
#if 0
        ANativeWindow_release(mWindow);
#endif
        mWindow = nullptr;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_setTextureCb(JNIEnv* env, jobject obj, jobject surfaceTexture)
{
    mWindow = ANativeWindow_fromSurface(env, surfaceTexture);
    auto xRes = ANativeWindow_getWidth(mWindow);
    auto yRes = ANativeWindow_getHeight(mWindow);
    getRenderVidFrame()->setOutDim(xRes, yRes);

    ANativeWindow_setBuffersGeometry(mWindow, xRes, yRes, WINDOW_FORMAT_RGBA_8888);

    LOGW("NativeWindowType:%p\n", mWindow);

    if (mIsCompressed)
    {
        getVideoDecoder()->init(mWindow);
        getVideoDecoder()->create(getVideoDecoder()->getCodecFourCC());
        getVideoDecoder()->configure();
    }
}
#else
namespace // #ifdef ANDROID_PLATFORM
{
    std::mutex mRenderMutex;
    std::unique_ptr<RenderVidFrame> mRenderVidFrame;
    std::unique_ptr<LinuxDecoder> mVidDecoder;
}
#endif // #ifdef ANDROID_PLATFORM

//////////////////////////////////////////////////
// RenderVidFrame implementation

void RenderVidFrame::onRender(std::unique_ptr<uint8_t[]> frameBytes, size_t size)
{
#ifdef ANDROID_PLATFORM
    DBG_RENDER("onRender, mWindow:%p, frameBytes:%p, size:%d\n", mWindow, frameBytes.get(), size);
    if (!frameBytes || !size)
    {
        //nothing to do
        return;
    }

    {
        std::lock_guard lk(mRenderMutex);
        if (!::mWindow)
        {
            requestTexture(this, false);
        }
        if (::mWindow)
        {
            ANativeWindow_Buffer buffer;
            ARect inOutDirtyBounds;
            auto ret = ANativeWindow_lock(::mWindow, &buffer, &inOutDirtyBounds);
            if (0 != ret)
            {
                LOGE("ANativeWindow_lock fail:0x%lx\n", ret);
            }
            else
            {
                // Copy RGBA data to buffer
                for (int y = 0; y < buffer.height; y++)
                {
                    uint8_t* dst = (uint8_t*) buffer.bits + y * buffer.stride * 4;
                    uint8_t* src = frameBytes.get() + y * mXres * 4;
                    memcpy(dst, src, buffer.width * 4);
                }
                // LOGW("buffer.width:%d, buffer.height:%d\n", buffer.width, buffer.height);

                // Unlock ANativeWindow
                ANativeWindow_unlockAndPost(::mWindow);
            }
        }
    }
#endif
}

std::pair<unsigned, unsigned> RenderVidFrame::getOutDim() const
{
    return {mXres, mYres};
}

void RenderVidFrame::setDecoder(Video::Decoder* decoder)
{
    mVideoDecoder = decoder;
}

//////////////////////////////////////////////////
// RenderVidFrame singleton

RenderVidFrame* getRenderVidFrame()
{
    if (!mRenderVidFrame)
    {
        mRenderVidFrame.reset(new RenderVidFrame());
    }
    return mRenderVidFrame.get();
}

Video::Decoder* getVideoDecoder()
{
    if (!mVidDecoder)
    {
#ifdef ANDROID_PLATFORM
        mVidDecoder.reset(new AndroidDecoder(requestTexture));
#else
#ifdef LINUX_PLATFORM
        mVidDecoder.reset(new LinuxDecoder(nullptr));
#else
        assert(false && "Setup video decoder for the platform");
#endif
#endif
    }
    return mVidDecoder.get();
}