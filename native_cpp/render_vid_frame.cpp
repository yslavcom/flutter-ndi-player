#include "render_vid_frame.hpp"
#include "common/logger.hpp"

#include <jni.h>

#include <functional>
#include <chrono>

namespace
{
    JavaVM *m_jvm;
    jobject gInterfaceObject;

    const char* path = "com/example/ndi_player/Render";

    using callback_t = std::function<void(const void*, int)>;
    callback_t callback;

    void set_callback(callback_t cb)
    {
        callback = cb;
    }

    std::unique_ptr<RenderVidFrame> mRenderVidFrame;
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
            jmethodID method = env->GetStaticMethodID(interfaceClass, "onCallback", "(Ljava/nio/ByteBuffer;)V");

            env->CallStaticVoidMethod(interfaceClass, method, byteBufferObj);
            m_jvm->DetachCurrentThread();
        }
    });

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_1player_RenderHelper_cleanup(JNIEnv *env_in, jobject instance, jint ptr)
{
    LOGW("cleanup from Kotlin:%d\n", ptr);

    // getRenderVidFrame()->cleanup(ptr);
}

RenderVidFrame* getRenderVidFrame()
{
    if (!mRenderVidFrame)
    {
        mRenderVidFrame.reset(new RenderVidFrame());
    }
    return mRenderVidFrame.get();
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

    auto releasedPtr = frameBytes.release();
    mCleanupMemPtr.emplace(releasedPtr);

    if (callback)
    {
        using namespace std::chrono;
        auto start = high_resolution_clock::now();

        callback(releasedPtr, size);

        auto now = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(now - start);
        LOGW("callback-dur:%d\n", duration.count());
    }
}

void RenderVidFrame::cleanup(uint8_t* ptr)
{
    //TODO: call 'cleanup' from kotlin when done rendering the frame
    auto it = mCleanupMemPtr.find(ptr);
    if (it != mCleanupMemPtr.cend())
    {
        delete [] *it;
    }
}
