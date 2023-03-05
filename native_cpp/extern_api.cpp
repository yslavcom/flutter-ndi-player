#include "ndi-rx/ndi-rx.hpp"

#include <memory>
#include <mutex>

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
void scanNdiSources()
{
    getInstanceRx()->start();
    getInstanceRx()->scanNdiSources();
}
