#pragma once

#include "interfaces/player-if.hpp"
#include "common/frame-queue.hpp"
#include "window.hpp"

#include <map>
#include <string>
#include <thread>
#include <mutex>

class ScreenPlayer: public IPlayer
{
public:
    ScreenPlayer(FrameQueue::Queue& frameQueue);

    bool createWindow(std::string winName, unsigned xRes, unsigned yRes);
    void renderFrames();

    bool shouldClose();

    // IPlayer
    bool openPlayWindow(std::string winName, unsigned xRes, unsigned yRes) override;

private:
    std::mutex mPlayerMu;
    FrameQueue::Queue& mFrameQueue;
    static WindowsHandle* mWindowsHandle;
    std::thread mRenderThread;

    enum class StatusWindow
    {
        Idle,
        CreateWindow,
        Run,
        RequestClose,
        WaitClosed,
        Closed,
    };
    StatusWindow mStatusWindow;
    friend std::ostream &operator<<(std::ostream &os, ScreenPlayer::StatusWindow const &status);

    void onWindowClose();
    bool run();
    std::function<void()>mCreateNextWindow;
    unsigned mXres;
    unsigned mYres;
    std::string mWinName;

    class LastFrame
    {
    public:
        ~LastFrame()
        {
            releaseFrame();
        }

        operator bool() const
        {
            return mFrame != nullptr;
        }

        void releaseFrame()
        {
            if (mFrame)
            {
                auto release = mFrame->second;
                if (release)
                {
                    release(mFrame->first.opaque);
                }
            }
            mFrame.reset();
        }

        const FrameQueue::VideoFrame& getFrame()
        {
            return mFrame->first;
        }

        void reset(const FrameQueue::Frame& frame)
        {
            // release the previous frame
            releaseFrame();
            mFrame.reset(new FrameQueue::Frame(frame));
        }

    private:
        std::unique_ptr<FrameQueue::Frame> mFrame;
    };
    LastFrame mLastFrame;
};