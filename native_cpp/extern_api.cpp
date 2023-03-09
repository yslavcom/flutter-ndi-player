#include "ndi-rx/ndi-rx.hpp"

#include <memory>
#include <mutex>

#include <iostream>

#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

namespace
{
std::unique_ptr<NdiRx> mNdiRx;
std::once_flag mNdiRxInitFlag;

void buildInstanceRx()
{
    mNdiRx.reset(new NdiRx);
}

NdiRx* getInstanceRx()
{
    std::call_once(mNdiRxInitFlag, &buildInstanceRx);
    return mNdiRx.get();
}

} // anon namespace

EXPORT
int scanNdiSources()
{
    std::cout << __func__ << std::endl;

    getInstanceRx()->start();
    getInstanceRx()->scanNdiSources();
    return 0;
}
