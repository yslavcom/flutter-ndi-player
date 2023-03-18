#include "ndi-rx-scan/ndi-rx.hpp"
#include "ndi-rx/ndi-app.hpp"
#include "ndi_src_observer.hpp"

#include "DartApiDL/include/dart_api_dl.c"

#include <memory>
#include <mutex>
#include <vector>

#include <iostream>

#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

// Scan Singleton
class NdiRxScan
{
public:
    static NdiRx* getInstance()
    {
        std::call_once(mInitFlag, &NdiRxScan::buildInstance);
        return mNdiRx.get();
    }

private:
    static std::unique_ptr<NdiRx> mNdiRx;
    static std::once_flag mInitFlag;
    static void buildInstance()
    {
        mNdiRx.reset(new NdiRx);
    }
};

std::unique_ptr<NdiRx> NdiRxScan::mNdiRx;
std::once_flag NdiRxScan::mInitFlag;

auto Scan = NdiRxScan::getInstance();

// Start Program Rx Singleton
class NdiRxProg
{
public:
    static NdiApp* getInstance()
    {
        std::call_once(mInitFlag, &NdiRxProg::buildInstance);
        return mNdiApp.get();
    }

private:
    static std::unique_ptr<NdiApp> mNdiApp;
    static std::once_flag mInitFlag;
    static void buildInstance()
    {
        mNdiApp.reset(new NdiApp);
    }
};

std::unique_ptr<NdiApp> NdiRxProg::mNdiApp;
std::once_flag NdiRxProg::mInitFlag;

auto ProgramRx = NdiRxProg::getInstance();

namespace
{
NdiSrcObserver mNdiSrcObserver;

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
            if ((mCharLen + el.size() + END_STRING_LEN) < mSourcesChars.size())
            {
                memcpy(&mSourcesChars[mCharLen], el.c_str(), el.size());
                mSourcesChars[mCharLen + el.size()] = '\r';
                mSourcesChars[mCharLen + el.size() + 1] = '\n';
                mCharLen += (el.size() + END_STRING_LEN);
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
    static constexpr unsigned END_STRING_LEN = 2;
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

    NdiApp::Quality mProgramQuality = NdiApp::Quality::High;

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
void startProgram(int64_t progrIdx)
{
    std::cout << __func__ << progrIdx << std::endl;

    ProgramRx->createReceiver("", "", mProgramQuality);
}

EXPORT
int32_t scanNdiSources()
{
    mNdiSrcObserver.setup(notifyUI_NdiSourceChange);

    Scan->start();
    Scan->addObserver(&mNdiSrcObserver);
    Scan->scanNdiSources();
    return 0;
}
