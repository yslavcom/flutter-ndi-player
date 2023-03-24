#include <android_native_app_glue.h>
#include <jni.h>

extern "C" {

void handle_cmd(android_app *pApp, int32_t cmd)
{
}

int32_t sumUp(int32_t a, int32_t b)
{
    return a + b;
}

void android_main(struct android_app *pApp) {
    pApp->onAppCmd = handle_cmd;

    int events;
    android_poll_source *pSource = nullptr;
    do {
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0) {
            if (pSource) {
                pSource->process(pApp, pSource);
            }
        }
    } while (!pApp->destroyRequested);
}
} // extern "C"