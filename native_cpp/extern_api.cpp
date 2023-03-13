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

std::vector<std::string> mSources;
struct CharFromSources
{
    CharFromSources() = default;
    CharFromSources(std::vector<std::string> sources)
    {
        mCharLen = 0;
        for (auto& el : sources)
        {
            if ((mCharLen + el.size() + NULL_CHAR_LEN) < mSourcesChars.size())
            {
                memcpy(&mSourcesChars[mCharLen], el.c_str(), el.size());
                mSourcesChars[mCharLen + el.size()] = '\0';
                mCharLen += (el.size() + NULL_CHAR_LEN);
            }
            else
            {
                std::cerr << "Failed to fit a source name:" << el << std::endl;
            }
        }
    }

    // The strings are copied in the single array and are seprated by 0.
    // Pre-allocate long enough array for storing the NDI source names.
    // We do not want to change it's location dynamically because the contents mightbe updated at any moment.
    std::array<uint8_t, 1024> mSourcesChars;
    unsigned mCharLen;
    static constexpr unsigned NULL_CHAR_LEN = 1;
};
CharFromSources mCharFromSources;

void sendMsgToFlutter(std::vector<std::string> sources)
{
    if (DartApiMessagePort == -1)
    {
        return;
    }
    mSources = sources;
    mCharFromSources = std::move(CharFromSources{sources});

    Dart_CObject obj;
    obj.type = Dart_CObject_kTypedData;
    obj.value.as_typed_data.type = Dart_TypedData_kUint8;
    obj.value.as_typed_data.length = mCharFromSources.mCharLen;
    obj.value.as_typed_data.values = mCharFromSources.mSourcesChars.data();
    Dart_PostCObject_DL(DartApiMessagePort, &obj);
}

} // anon namespace

EXPORT
int64_t initializeApiDLData(void *data)
{
    return Dart_InitializeApiDL(data);
}

EXPORT
void setDartApiMessagePort(int64_t port)
{
    DartApiMessagePort = port;
}

EXPORT
int32_t notifyUI_NdiSourceChange(std::vector<std::string> sources)
{
    sendMsgToFlutter(sources);
    return sources.size();
}

EXPORT
int32_t scanNdiSources()
{
    mNdiSrcObserver.setup(notifyUI_NdiSourceChange);

    getInstanceRx()->start();
    getInstanceRx()->addObserver(&mNdiSrcObserver);
    getInstanceRx()->scanNdiSources();
    return 0;
}
