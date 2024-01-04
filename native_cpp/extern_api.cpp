#include "ndi-rx-scan/ndi-rx.hpp"
#include "ndi-rx/ndi-app.hpp"
#include "ndi_src_observer.hpp"
#include "ndi_input_packet_observer.hpp"
#include "player/render_vid_frame.hpp"
#include "player/player.hpp"
#include "player/codec.hpp"

#include  "interfaces/input-control.hpp"
#include  "rx-frames-controller/rx-frame-controller.hpp"

#include "DartApiDL/include/dart_api_dl.c"

#include "common/logger.hpp"
#include "common/frame-queue.hpp"
#include "common/custom_thread.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <iostream>

#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

namespace
{

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
NdiSrcObserver mNdiSrcObserver;

// Start Program Rx Singleton
class NdiRxProg: public InputControl
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
std::unique_ptr<CustomThread> mCapturePacketsThread;

std::optional<int64_t> mCurrentProgramIdx;

std::mutex mFrameRxMutex;
FrameQueue::VideoRx mVideoRxQueue(mFrameRxMutex);
FrameQueue::AudioRx mAudioRxQueue(mFrameRxMutex);

NdiInputPacketsObserver mNdiInputPacketsObserver(mVideoRxQueue, mAudioRxQueue);

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
                LOGE("Failed to fit a source name:%s\n", el.c_str());
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
int64_t DartApiMessagePort = -1;

void sendMsgToFlutter(std::vector<std::string> sources)
{
    if (DartApiMessagePort == -1)
    {
        return;
    }
    mSources = sources;
    mCharFromSources = CharFromSources{sources};

    Dart_CObject obj;
    obj.type = Dart_CObject_kTypedData;
    obj.value.as_typed_data.type = Dart_TypedData_kUint8;
    obj.value.as_typed_data.length = mCharFromSources.mCharLen;
    obj.value.as_typed_data.values = mCharFromSources.mSourcesChars.data();
    Dart_PostCObject_DL(DartApiMessagePort, &obj);
}

NdiApp::Quality mProgramQuality = NdiApp::Quality::High;

std::unique_ptr<Player> mPlayer;

class NdiInputControl: public InputControl
{
    // InputControl interface
    virtual void restart() override
    {
        ProgramRx->requestKeyFrame();
    }
};
NdiInputControl mNdiInputControl;
RxFrameController mRxFrameController(mVideoRxQueue, mAudioRxQueue, &mNdiInputControl);
std::unique_ptr<CustomThread> mRxFrameControllerThread;
std::mutex mVideoDecoderFrameMutex;
FrameQueue::VideoRx mVidFramesToDecode(mVideoDecoderFrameMutex);
FrameQueue::VideoRx mVidFramesDecoded(mVideoDecoderFrameMutex);
} // anonymous namespace

EXPORT
int64_t initializeApiDLData(void *data)
{
    LOGW("%s, %p\n", __func__, data);

    return Dart_InitializeApiDL(data);
}

EXPORT
void setDartApiMessagePort(int64_t port)
{
    LOGW("%s:%ld\n", __func__, port);
    DartApiMessagePort = port;
}

EXPORT
int32_t notifyUI_NdiSourceChange(std::vector<std::string> sources)
{
    LOGW("%s, count:%d\n", __func__, sources.size());
    sendMsgToFlutter(sources);
    return sources.size();
}

EXPORT
void stopProgram(int64_t progrIdx)
{
    (void)progrIdx;

    LOGW("stopProgram\n");

    mCapturePacketsThread = nullptr;
    mRxFrameControllerThread = nullptr;

    mRxFrameController.uninstallVideoFrameObs(mPlayer.get());
    mRxFrameController.uninstallAudioFrameObs(mPlayer.get());

    mPlayer = nullptr;
    mVidFramesToDecode.flush();
}

static void restartProgramResources()
{
    mPlayer.reset(new Player);
    mPlayer->setRenderObserver(getRenderVidFrame());
    auto videoDecoder = getVideoDecoder();
    videoDecoder->terminate();
    videoDecoder->setVidFramesToDecode(&mVidFramesToDecode);
    videoDecoder->setDecodedFramesQueue(&mVidFramesDecoded);
    mPlayer->setDecoder(videoDecoder);
    getRenderVidFrame()->setDecoder(videoDecoder);
    mRxFrameController.installVideoFrameObs(mPlayer.get());
    mRxFrameController.installAudioFrameObs(mPlayer.get());

    mPlayer->reStart();
}

EXPORT
void startProgram(int64_t progrIdx)
{
    LOGW("%s:%ld\n", __func__, progrIdx);

    if (mCurrentProgramIdx)
    {
        stopProgram(*mCurrentProgramIdx);
    }
    mCurrentProgramIdx = progrIdx;

    auto name = Scan->getSourceName(progrIdx);
    auto url = Scan->getSourceUrl(progrIdx);

    mNdiInputPacketsObserver.clear();
    ProgramRx->addObserver(&mNdiInputPacketsObserver);

    if (ProgramRx->createReceiver(name, url, mProgramQuality))
    {
        restartProgramResources();

        mCapturePacketsThread.reset(new CustomThread());
        mCapturePacketsThread->start("mCapturePacketsThread", [](const bool stop){
                (void)stop;
                ProgramRx->capturePackets();
            });


        mRxFrameControllerThread.reset(new CustomThread());
        mRxFrameControllerThread->start("mRxFrameControllerThread", [](const bool stop){
            (void)stop;
            mRxFrameController.run();
        });
    }
}

EXPORT
int32_t scanNdiSources()
{
    LOGW("%s\n", __func__);

    mNdiSrcObserver.setup(notifyUI_NdiSourceChange);

    Scan->start();
    Scan->addObserver(&mNdiSrcObserver);
    Scan->scanNdiSources();
    return 0;
}

enum
{
    kVideoQueueType = 0,
    kAudioQueueType = 1,
};

EXPORT
int getOverflowCount(int type)
{
    if (type == kVideoQueueType)
    {
        return (int)mVideoRxQueue.getOverflowCount();
    }
    else if (type == kAudioQueueType)
    {
        return (int)mAudioRxQueue.getOverflowCount();
    }
    return 0;
}

EXPORT
int getRxQueueLen(int type)
{
    if (type == kVideoQueueType)
    {
        return mVideoRxQueue.getCount();
    }
    else if (type == kAudioQueueType)
    {
        return mAudioRxQueue.getCount();
    }
    return 0;
}

enum
{
    kGetVidAudFramesCount = 0,
    kGetSourceProgramInfo = 1,
};

EXPORT
void* getArray(int type, int* size)
{
    if (!size)
    {
        return nullptr;
    }
    else if (type == kGetVidAudFramesCount)
    {
        auto count = mNdiInputPacketsObserver.getRxFrameCount();
        auto ptr = (int*)malloc(sizeof(int)*2);
        if (ptr)
        {
            ptr[0] = count.first;
            ptr[1] = count.second;
        }
        *size = 2;
        return (void*)ptr;
    }
    return nullptr;
}

// all words as int32_t
// args => [size, length of args message including 'type', count in words], [type], ...[etc]

EXPORT
void* getArrayArgs(void* args, int* size)
{
    if (!args || !size)
    {
        return nullptr;
    }

    int32_t argsSize = *((int32_t*)args);
    if (argsSize < 1)
    {
        return nullptr;
    }

    int32_t type = *((int32_t*)args+1);

    if (type == kGetVidAudFramesCount)
    {
        return getArray(kGetVidAudFramesCount, size);
    }
    else if (type == kGetSourceProgramInfo)
    {
        // Retrieve complete info on NDI Source
        if (argsSize == 2)
        {
            int32_t sourceIdx = *((int32_t*)args+2);
            (void)sourceIdx;
        }

    }
    return nullptr;
}

EXPORT
void freeArray(void* addr)
{
    if (addr)
    {
        free(addr);
    }
}