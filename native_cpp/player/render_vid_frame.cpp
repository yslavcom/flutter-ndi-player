#include "render_vid_frame.hpp"
#include "android-codec.hpp"
#include "common/logger.hpp"

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>

#include <functional>
#include <chrono>
#include <memory>

namespace
{
// candidate to be removed
    JavaVM *m_jvm;
    jobject gInterfaceObject;

    const char* pathTex = "com/example/ndi_player/Texture";
    using ReqTextCb_t = std::function<void()>;
    ReqTextCb_t reqTextCb;

    void setReqTextCb(ReqTextCb_t cb)
    {
        reqTextCb = cb;
    }

    ANativeWindow* mWindow = nullptr;

    std::mutex mRenderMutex;
    std::unique_ptr<RenderVidFrame> mRenderVidFrame;
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
    {
        std::lock_guard lk(mRenderMutex);
        getRenderVidFrame()->setOutDim(0, 0);
//        ANativeWindow_release(mWindow);
        mWindow = nullptr;
    }
    LOGW("Clear surface\n");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_setTextureCb(JNIEnv* env, jobject obj, jobject surfaceTexture)
{
    auto resolution = getRenderVidFrame()->getOutDim();
    mWindow = ANativeWindow_fromSurface(env, surfaceTexture);
    ANativeWindow_setBuffersGeometry(mWindow, resolution.first, resolution.second, WINDOW_FORMAT_RGBA_8888);

    LOGW("NativeWindowType:%p\n", mWindow);

    mVidDecoder.reset(new AndroidDecoder(resolution.first, resolution.second, mWindow));
    mVidDecoder->create();
    mVidDecoder->configure();
}

//////////////////////////////////////////////////
// RenderVidFrame implementation

void RenderVidFrame::onRender(std::unique_ptr<uint8_t[]> frameBytes, size_t size)
{
    if (!frameBytes || !size)
    {
        //nothing to do
        return;
    }

    {
        std::lock_guard lk(mRenderMutex);
        if (!mWindow)
        {
            if (auto [w, h] = getOutDim(); w != 0 && h != 0)
            {
                if (reqTextCb)
                {
                    reqTextCb();
                }
            }
        }
        if (mWindow)
        {
            ANativeWindow_Buffer buffer;
            ARect inOutDirtyBounds;
            auto ret = ANativeWindow_lock(mWindow, &buffer, &inOutDirtyBounds);
            if (0 != ret)
            {
                LOGE("ANativeWindow_lock fail:0x%lx\n", ret);
            }
            else
            {
#if 1
                // Copy RGBA data to buffer
                for (int y = 0; y < buffer.height; y++)
                {
                    uint8_t* dst = (uint8_t*) buffer.bits + y * buffer.stride * 4;
                    uint8_t* src = frameBytes.get() + y * mXres * 4;
                    memcpy(dst, src, buffer.width * 4);
                }
#endif
                LOGW("buffer.width:%d, buffer.height:%d\n", buffer.width, buffer.height);
                // Unlock ANativeWindow
                ANativeWindow_unlockAndPost(mWindow);
//                // Release ANativeWindow
//                ANativeWindow_release(mWindow);
            }
        }
    }

#if 0
    auto releasedPtr = frameBytes.release();
    mCleanupMemPtr.emplace(releasedPtr);
    if (callback)
    {
#if 0
        using namespace std::chrono;
        auto start = high_resolution_clock::now();
#endif
        LOGW("releasedPtr:%p\n", releasedPtr);
        callback(releasedPtr, size);
#if 0
        auto now = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(now - start);
        LOGW("callback-dur:%d\n", duration.count());
#endif
    }
#endif
}

std::pair<unsigned, unsigned> RenderVidFrame::getOutDim() const
{
    return {mXres, mYres};
}

void RenderVidFrame::cleanup(uint8_t* ptr)
{
    //TODO: call 'cleanup' from kotlin when done rendering the frame
    auto it = mCleanupMemPtr.find(ptr);
    if (it != mCleanupMemPtr.cend())
    {
        LOGW("Cleanup:%p\n", ptr);
        delete [] *it;
    }
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
