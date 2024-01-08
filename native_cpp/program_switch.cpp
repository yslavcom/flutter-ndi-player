#include "program_switch.hpp"

#define _DBG_PROG_SWITCH

#ifdef _DBG_PROG_SWITCH
    #define DBG_PROG_SWITCH(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PROG_SWITCH(format, ...)
#endif

namespace Monitor
{
namespace {
// Scan Singleton

#if 0
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
#endif

#if 0
std::unique_ptr<CustomThread> mCapturePacketsThread;

std::mutex mFrameRxMutex;
FrameQueue::VideoRx mVideoRxQueue(mFrameRxMutex);
FrameQueue::AudioRx mAudioRxQueue(mFrameRxMutex);


NdiInputPacketsObserver mNdiInputPacketsObserver(mVideoRxQueue, mAudioRxQueue);

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

// consider removing it
FrameQueue::VideoRx mVidFramesDecoded(mVideoDecoderFrameMutex);

#endif

} // anon namespace

ProgramSwitch::ProgramSwitch(NdiSourceChangeNotify ndiSourceChangeNotify, Msg::Queue& msgQueue)
    : mCurrentProgramIdx{}
    , mProgramQuality(NdiApp::Quality::High)
    , mNdiSourceChangeNotify(ndiSourceChangeNotify)
    , mVideoRxQueue(mFrameRxMutex)
    , mAudioRxQueue(mFrameRxMutex)
    , mNdiInputPacketsObserver(mVideoRxQueue, mAudioRxQueue)
    , mVidFramesToDecode(mVideoDecoderFrameMutex)
    , mVidFramesDecoded(mVideoDecoderFrameMutex)
    , mMsgQueue(msgQueue)
{
    Scan.reset(new NdiRx);
    mNdiSrcObserver.setup(mNdiSourceChangeNotify);
    Scan->addObserver(&mNdiSrcObserver);

    ProgramRx.reset(new NdiApp);
    mCapturePacketsThread.reset(new CustomThread);
    mNdiInputControl.reset(new NdiInputControl(*ProgramRx.get()));

    mRxFrameController.reset(new RxFrameController(mVideoRxQueue, mAudioRxQueue, mNdiInputControl.get()));

    mNdiInputPacketsObserver.setInformCompressedTypeCallback([this](H26x::FourCcType vidFourCcType){
        onInformCompressedType(vidFourCcType);
    });
}

void ProgramSwitch::reStartProgram()
{
    startProgramUnsafe();
}

void ProgramSwitch::startProgramUnsafe()
{
    if (!mCurrentProgramIdx) return;

    stopProgram();

    auto name = Scan->getSourceName(*mCurrentProgramIdx);
    auto url = Scan->getSourceUrl(*mCurrentProgramIdx);

    mNdiInputPacketsObserver.clear();
    ProgramRx->addObserver(&mNdiInputPacketsObserver);

    if (ProgramRx->createReceiver(mProgramQuality))
    {
        ProgramRx->connect(name, url);
        restartProgramResources();

        mCapturePacketsThread.reset(new CustomThread());
        mCapturePacketsThread->start("mCapturePacketsThread", [this](const bool stop){
                (void)stop;
                ProgramRx->capturePackets();
            });


        mRxFrameControllerThread.reset(new CustomThread());
        mRxFrameControllerThread->start("mRxFrameControllerThread", [this](const bool stop){
            (void)stop;
            mRxFrameController->run();
        });
    }
}

void ProgramSwitch::startProgram(int64_t progrIdx)
{

    LOGW("%s:%ld\n", __func__, progrIdx);

    mCurrentProgramIdx = progrIdx;
    startProgramUnsafe();
}

void ProgramSwitch::stopProgram()
{
    LOGW("stopProgram\n");

    mCapturePacketsThread = nullptr;
    mRxFrameControllerThread = nullptr;

    mRxFrameController->uninstallVideoFrameObs(mPlayer.get());
    mRxFrameController->uninstallAudioFrameObs(mPlayer.get());

    mPlayer = nullptr;

    mVideoRxQueue.flush();
    mAudioRxQueue.flush();
    mVidFramesToDecode.flush();
}

void ProgramSwitch::restartProgramResources()
{
    mPlayer.reset(new Player([this](){
        ProgramSwitch::updateAboutChange();
    }));
    mPlayer->setRenderObserver(getRenderVidFrame());
    auto videoDecoder = getVideoDecoder();
    videoDecoder->terminate();
    videoDecoder->setVidFramesToDecode(&mVidFramesToDecode);
    videoDecoder->setDecodedFramesQueue(&mVidFramesDecoded);
    mPlayer->setDecoder(videoDecoder);
    getRenderVidFrame()->setDecoder(videoDecoder);
    mRxFrameController->installVideoFrameObs(mPlayer.get());
    mRxFrameController->installAudioFrameObs(mPlayer.get());

    mPlayer->reStart();
}

void ProgramSwitch::switchToUncompressed()
{
    // switch to uncompressed mode
    ProgramRx->disconnectReceiver();
    if (ProgramRx->createReceiverUcompressed(mProgramQuality))
    {
        auto name = Scan->getSourceName(*mCurrentProgramIdx);
        auto url = Scan->getSourceUrl(*mCurrentProgramIdx);
        ProgramRx->connect(name, url);
    }
}

int32_t ProgramSwitch::scanNdiSources()
{
    LOGW("%s\n", __func__);

    Scan->start();
    Scan->scanNdiSources();
    return 0;
}

uint32_t ProgramSwitch::getVideoOverflowCount() const
{
    return mVideoRxQueue.getOverflowCount();
}

uint32_t ProgramSwitch::getAudioOverflowCount() const
{
    return mAudioRxQueue.getOverflowCount();
}

uint32_t ProgramSwitch::getVideoQueueLen() const
{
    return mVideoRxQueue.getOverflowCount();
}

uint32_t ProgramSwitch::getAudioQueueLen() const
{
    return mAudioRxQueue.getOverflowCount();
}

std::pair<unsigned, unsigned> ProgramSwitch::getRxFrameCount() const
{
    return mNdiInputPacketsObserver.getRxFrameCount();
}

void ProgramSwitch::updateAboutChange()
{
    // TODO: implement me
}

void ProgramSwitch::onInformCompressedType(H26x::FourCcType vidFourCcType)
{
    switch (vidFourCcType)
    {
    case H26x::FourCcType::Unknown:
        mMsgQueue.push(Msg::Type::SwitchToUncompressed);
    break;
    case H26x::FourCcType::H264:
    break;
    case H26x::FourCcType::Hevc:
    break;
    }
}

} // namespace Monitor
