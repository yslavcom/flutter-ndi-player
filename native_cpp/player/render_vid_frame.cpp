#include "render_vid_frame.hpp"
#include "common/logger.hpp"
#include "render.hpp"

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
#if 1
// candidate to be removed
    JavaVM *m_jvm;
    jobject gInterfaceObject;

    const char* path = "com/example/ndi_player/Render";

    using callback_t = std::function<void(const void*, int)>;
    callback_t callback;

    void set_callback(callback_t cb)
    {
        callback = cb;
    }
#endif

    std::mutex mRenderMutex;
    std::unique_ptr<RenderVid> mRender;

    std::unique_ptr<RenderVidFrame> mRenderVidFrame;
}

#if 1
// candidate to be removed
extern "C"
JNIEXPORT jint
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    m_jvm = vm;

    jclass cls = env->FindClass(path);
    jmethodID constr = env->GetMethodID(cls, "<init>", "()V");
    jobject obj = env->NewObject(cls, constr);
    gInterfaceObject = env->NewGlobalRef(obj);

//    m_callbackMethod = env->GetStaticMethodID(m_callbackClass, "onCallback", "(Ljava/nio/ByteBuffer;)V");

    // Save the callback function pointer
    set_callback([](const void* data, int length) {
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
            jobject byteBufferObj = env->NewDirectByteBuffer(const_cast<void*>(data), length);


            jclass interfaceClass = env->GetObjectClass(gInterfaceObject);
            jmethodID method = env->GetStaticMethodID(interfaceClass, "onCallback", "(Ljava/nio/ByteBuffer;J)V");

            env->CallStaticVoidMethod(interfaceClass, method, byteBufferObj, reinterpret_cast<jlong>(data));
            m_jvm->DetachCurrentThread();
        }
    });

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_RenderHelper_cleanup(JNIEnv *env_in, jobject instance, jlong ptr)
{
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);
    LOGW("cleanup from Kotlin:%p\n", data);
    getRenderVidFrame()->cleanup(data);
}
#endif

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_setTexture(JNIEnv* env, jobject obj, jobject surfaceTexture)
{
    auto window = ANativeWindow_fromSurface(env, surfaceTexture);
    int width = ANativeWindow_getWidth(window);
    int height = ANativeWindow_getHeight(window);

    {
        std::lock_guard lk(mRenderMutex);

        mRender = std::make_unique<RenderVid>();
        mRender->init(window);
        getRenderVidFrame()->setOutDim(width, height);
    }
    LOGW("NativeWindowType:%p, width:%d, height:%d\n", window, width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_TextureHelper_clearTexture(JNIEnv* env, jobject)
{
    {
        std::lock_guard lk(mRenderMutex);
        mRender = nullptr;
        getRenderVidFrame()->setOutDim(0, 0);
    }
    LOGW("Clear surface\n");
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
    //LOGW("onRender:%d\n", size);

    {
        std::lock_guard lk(mRenderMutex);
        if (mRender)
        {
            mRender->render(frameBytes.get(), mXres, mYres);
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
