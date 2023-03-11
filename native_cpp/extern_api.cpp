#include "ndi-rx/ndi-rx.hpp"
#include "ndi_src_observer.hpp"

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

int32_t (*ndiSourceChange)(int32_t) = nullptr;

} // anon namespace

EXPORT
int32_t notifyUI_NdiSourceChange(int32_t count)
{
    if (ndiSourceChange)
    {
        ndiSourceChange(count);
    }
    return count;
}

EXPORT
void notifyUI_NdiSourceChange_CbRegister(int32_t (*cb)(int32_t))
{
    ndiSourceChange = cb;
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
