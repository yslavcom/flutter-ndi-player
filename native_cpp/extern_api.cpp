#include "ndi-rx/ndi-rx.hpp"
#include "ndi_src_observer.hpp"

#include "DartApiDL/include/dart_api_dl.c"

#include <memory>
#include <mutex>
#include <vector>

#include <iostream>

#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

namespace
{
std::unique_ptr<NdiRx> mNdiRx;
std::once_flag mNdiRxInitFlag;

NdiSrcObserver mNdiSrcObserver;

void buildInstanceRx()
{
    mNdiRx.reset(new NdiRx);
}

NdiRx* getInstanceRx()
{
    std::call_once(mNdiRxInitFlag, &buildInstanceRx);
    return mNdiRx.get();
}

int64_t DartApiMessagePort = -1;

// this will send long integer to dart receiver port as a message
void sendMsgToFlutter(int64_t msg)
{
    if (DartApiMessagePort == -1)
    {
        return;
    }
    Dart_CObject obj;
    obj.type = Dart_CObject_kInt64;
    obj.value.as_int64 = msg;
    Dart_PostCObject_DL(DartApiMessagePort, &obj);
}

} // anon namespace

EXPORT
int64_t initializeApiDLData(void *data)
{
    std::cout << __func__ << ":" << data << std::endl;
    return Dart_InitializeApiDL(data);
}

EXPORT
void setDartApiMessagePort(int64_t port)
{
    std::cout << __func__ << ":" << port << std::endl;
    DartApiMessagePort = port;
}

EXPORT
int32_t notifyUI_NdiSourceChange(int32_t count)
{
    sendMsgToFlutter(count);
    return count;
}

EXPORT
int32_t scanNdiSources()
{
    std::cout << __func__ << std::endl;

    mNdiSrcObserver.setup(notifyUI_NdiSourceChange);

    getInstanceRx()->start();
    getInstanceRx()->addObserver(&mNdiSrcObserver);
    getInstanceRx()->scanNdiSources();
    return 0;
}
