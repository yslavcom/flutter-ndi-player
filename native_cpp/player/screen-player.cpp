#include "screen-player.hpp"

#include <iostream>

WindowsHandle* ScreenPlayer::mWindowsHandle = nullptr;

std::ostream &operator<<(std::ostream &os, ScreenPlayer::StatusWindow const &status) {
    switch(status)
    {
    case ScreenPlayer::StatusWindow::Idle:
        return os << "StatusWindow::Idle";
    case ScreenPlayer::StatusWindow::CreateWindow:
        return os << "StatusWindow::CreateWindow";
    case ScreenPlayer::StatusWindow::Run:
        return os << "StatusWindow::Run";
    case ScreenPlayer::StatusWindow::RequestClose:
        return os << "StatusWindow::RequestClose";
    case ScreenPlayer::StatusWindow::WaitClosed:
        return os << "StatusWindow::WaitClosed";
    case ScreenPlayer::StatusWindow::Closed:
        return os << "StatusWindow::Closed";
    }
    return os;
}

ScreenPlayer::ScreenPlayer(FrameQueue::Queue& frameQueue)
    : mStatusWindow(StatusWindow::Idle)
    , mFrameQueue(frameQueue)
{
    mRenderThread = std::thread([this]()
    {
#if 1
        for(;;)
        {
            run();
        }
#else
        bool ret = false;
        do
        {
            ret = run();
        }
        while(ret);
#endif
//        std::cout << "exit mRenderThread" << std::endl;
    });
}

bool ScreenPlayer::createWindow(std::string winName, unsigned xRes, unsigned yRes)
{
    mWindowsHandle->create(winName, xRes, yRes
        , [this](){
            onWindowClose();
            }
        );
    mWindowsHandle->setContext(winName);
    mWindowsHandle->setFocus(winName);

    std::cout << "createWindow:" << winName << std::endl;
    return true;
}

void ScreenPlayer::renderFrames()
{
    bool empty = mFrameQueue.getCount() == 0;
    if (!empty)
    {
        FrameQueue::Frame frame;
        if (mFrameQueue.read(frame))
        {
            mLastFrame.reset(frame);
        }
    }

    if (mLastFrame)
    {
   //     std::cout << "frame:" << (uintptr_t)mLastFrame.getFrame().opaque << std::endl;

        unsigned destWinIdx = 0;
        mWindowsHandle->renderFrame(mLastFrame.getFrame(), destWinIdx);
    }
}

bool ScreenPlayer::run()
{
    std::lock_guard lk(mPlayerMu);
    switch (mStatusWindow)
    {
    case StatusWindow::Idle:
        break;
    case StatusWindow::CreateWindow:
        if (mCreateNextWindow)
        {
            mCreateNextWindow();
            mCreateNextWindow = nullptr;
            mStatusWindow = StatusWindow::Run;
        }
        else
        {
            return false;
        }
        break;
    case StatusWindow::Run:
        if(!shouldClose())
        {
            renderFrames();
        }
        else
        {
            mStatusWindow = StatusWindow::Closed;
        }
        break;
    case StatusWindow::RequestClose:
        mWindowsHandle->closeAllWindows();
        mStatusWindow = StatusWindow::WaitClosed;
        break;
    case StatusWindow::WaitClosed:
        mStatusWindow = StatusWindow::Closed;
        break;
    case StatusWindow::Closed:
        if (mCreateNextWindow)
        {
            mCreateNextWindow();
            mCreateNextWindow = nullptr;
        }
        else
        {
            std::cout << "exit loop" << std::endl;
            mStatusWindow = StatusWindow::Idle;
            return false;
        }
        break;
    }

    mWindowsHandle->pollEvents();

    return true;
}

bool ScreenPlayer::openPlayWindow(std::string winName, unsigned xRes, unsigned yRes)
{
    std::cout << "openPlayWindow\n";

    if (!mWindowsHandle)
    {
        mWindowsHandle = WindowsHandle::getInstance();
    }

    auto delayedWindowCreate = [this](){
        std::cout << "delayedWindowCreate name:" << mWinName << std::endl;
        createWindow(mWinName, mXres, mYres);
        mStatusWindow = StatusWindow::Run;
    };

    {
        std::lock_guard lk(mPlayerMu);

        std::cout << mStatusWindow << std::endl;

        mXres = xRes;
        mYres = yRes;
        mWinName = winName;

        switch (mStatusWindow)
        {
        case StatusWindow::Idle:
            mStatusWindow = StatusWindow::CreateWindow;
            break;
        case StatusWindow::CreateWindow:
            break;
        case StatusWindow::Run:
            mStatusWindow = StatusWindow::RequestClose;
            break;
        case StatusWindow::RequestClose:
            break;
        case StatusWindow::WaitClosed:
            mStatusWindow = StatusWindow::Closed;
            break;
        case StatusWindow::Closed:
            break;
        }

        mCreateNextWindow = delayedWindowCreate;
    }

    return true;
}

void ScreenPlayer::onWindowClose()
{
    mStatusWindow = StatusWindow::Closed;
    mWindowsHandle->closeAllWindows();
}

bool ScreenPlayer::shouldClose()
{
    auto ret = mWindowsHandle->shouldClose();
    if (ret)
    {
        std::cout << "should close\n";
    }
    return ret;
}
