#pragma once

#include "common/frame-queue.hpp"

#include <string>
#include <map>
#include <memory>
#include <vector>

class Window;
class Texture2D;

using OnCloseCallback = std::function<void()>;

class WindowsHandle
{
public:
    WindowsHandle() = default;
    static WindowsHandle* getInstance();
    ~WindowsHandle();
    WindowsHandle(WindowsHandle&&) = delete;
    WindowsHandle(const WindowsHandle&) = delete;
    WindowsHandle& operator=(const WindowsHandle&) = delete;
    WindowsHandle& operator=(WindowsHandle&&) = delete;

    bool create(std::string name, unsigned xRes, unsigned yRes, OnCloseCallback onCloseCb);

    void renderFrame(const FrameQueue::VideoFrameStr& video, unsigned destWinIdx);

    void dbgFilleBuffer();

private:

};
