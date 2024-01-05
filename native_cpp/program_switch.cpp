#include "program_switch.hpp"

#include "ndi-rx-scan/ndi-rx.hpp"
#include "ndi-rx/ndi-app.hpp"
#include "ndi_input_packet_observer.hpp"
#include "player/render_vid_frame.hpp"
#include "player/player.hpp"
#include "player/codec.hpp"

#include  "interfaces/input-control.hpp"
#include  "rx-frames-controller/rx-frame-controller.hpp"

#include "common/logger.hpp"
#include "common/frame-queue.hpp"
#include "common/custom_thread.hpp"

namespace Monitor
{
namespace {
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

} // anon namespace

ProgramSwitch::ProgramSwitch(NdiSourceChangeNotify ndiSourceChangeNotify)
    : mNdiSourceChangeNotify(ndiSourceChangeNotify)
{}

void ProgramSwitch::reStartProgram()
{
    std::lock_guard lk(m);
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

void ProgramSwitch::startProgram(int64_t progrIdx)
{

    LOGW("%s:%ld\n", __func__, progrIdx);

    std::lock_guard lk(m);

    mCurrentProgramIdx = progrIdx;
    startProgramUnsafe();
}

void ProgramSwitch::stopProgram()
{
    LOGW("stopProgram\n");

    mCapturePacketsThread = nullptr;
    mRxFrameControllerThread = nullptr;

    mRxFrameController.uninstallVideoFrameObs(mPlayer.get());
    mRxFrameController.uninstallAudioFrameObs(mPlayer.get());

    mPlayer = nullptr;

    mVideoRxQueue.flush();
    mAudioRxQueue.flush();
    mVidFramesToDecode.flush();
}

void ProgramSwitch::restartProgramResources()
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

int32_t ProgramSwitch::scanNdiSources()
{
    LOGW("%s\n", __func__);

    mNdiSrcObserver.setup(mNdiSourceChangeNotify);

    Scan->start();
    Scan->addObserver(&mNdiSrcObserver);
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

} // namespace Monitor