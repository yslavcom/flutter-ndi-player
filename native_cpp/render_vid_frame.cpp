#include "render_vid_frame.hpp"
#include "common/logger.hpp"

#include <jni.h>

#include <functional>

namespace
{
    //typedef void (*callback_t)(const void*, int);
    using callback_t = std::function<void(const void*, int)>;
    callback_t callback;

    void set_callback(callback_t cb)
    {
        callback = cb;
    }

    std::unique_ptr<RenderVidFrame> mRenderVidFrame;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_ndi_player_RegisterRender_registerCallback(JNIEnv *env, jobject instance, jobject callbackObj)
{
    jclass callbackClass = env->GetObjectClass(callbackObj);
    jmethodID callbackMethod = env->GetMethodID(callbackClass, "onCallback", "(Ljava/nio/ByteBuffer;)V");

    // Save the callback function pointer
    set_callback([env, callbackObj, callbackMethod](const void* data, int length) {
        jobject byteBufferObj = env->NewDirectByteBuffer(const_cast<void*>(data), length);
        env->CallVoidMethod(callbackObj, callbackMethod, byteBufferObj);
    });
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

    LOGW("Render:%d\n", size);

    auto releasedPtr = frameBytes.release();
    mCleanupMemPtr.emplace(releasedPtr);

    if (callback)
    {
        callback(releasedPtr, size);
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