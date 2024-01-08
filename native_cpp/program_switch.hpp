#pragma once

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
#include "common/msg_queue.hpp"

#include "ndi_src_observer.hpp"
#include "player/sps_pps_parser.hpp"

#include <cstdint>
#include <optional>
#include <mutex>
#include <functional>
#include <string>
#include <memory>

namespace Monitor
{
using NdiSourceChangeNotify = NdiSrcObserver::UiUpdateCb;

class ProgramSwitch
{
public:
    ProgramSwitch(NdiSourceChangeNotify ndiSourceChangeNotify, Msg::Queue& msgQueue);

    void startProgram(int64_t progrIdx);
    void stopProgram();
    int32_t scanNdiSources();
    uint32_t getVideoOverflowCount() const;
    uint32_t getAudioOverflowCount() const;
    uint32_t getVideoQueueLen() const;
    uint32_t getAudioQueueLen() const;
    std::pair<unsigned, unsigned> getRxFrameCount() const;
    void reStartProgram();
    void switchToUncompressed();

private:
    std::optional<int64_t> mCurrentProgramIdx;
    NdiApp::Quality mProgramQuality;

    void restartProgramResources();
    void startProgramUnsafe();

    void updateAboutChange();

    NdiSourceChangeNotify mNdiSourceChangeNotify;

    void onInformCompressedType(H26x::FourCcType vidFourCcType);

    std::unique_ptr<NdiRx> Scan;
    NdiSrcObserver mNdiSrcObserver;
    std::unique_ptr<NdiApp> ProgramRx;

    std::unique_ptr<CustomThread> mCapturePacketsThread;

    std::mutex mFrameRxMutex;
    FrameQueue::VideoRx mVideoRxQueue;
    FrameQueue::AudioRx mAudioRxQueue;
    NdiInputPacketsObserver mNdiInputPacketsObserver;

    std::unique_ptr<Player> mPlayer;

    class NdiInputControl: public InputControl
    {
    public:
        NdiInputControl(NdiApp& ndiApp)
            : mNdiApp(ndiApp)
        {}

        virtual ~NdiInputControl(){}

        // InputControl interface
        virtual void restart() override
        {
            mNdiApp.requestKeyFrame();
        }
    private:
        NdiApp& mNdiApp;
    };

    std::unique_ptr<NdiInputControl> mNdiInputControl;

    std::unique_ptr<RxFrameController> mRxFrameController;
    std::unique_ptr<CustomThread> mRxFrameControllerThread;
    std::mutex mVideoDecoderFrameMutex;
    FrameQueue::VideoRx mVidFramesToDecode;

    // consider removing it
    FrameQueue::VideoRx mVidFramesDecoded;

    Msg::Queue& mMsgQueue;
};
}