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
    bool rename(std::string oldName, std::string newName);
    bool closeAllWindows();
    bool setContext(std::string name);
    bool setFocus(std::string name);

    bool shouldClose();
    void renderFrame(const FrameQueue::VideoFrame& video, unsigned destWinIdx);

    void pollEvents();

    void dbgFilleBuffer();

private:
    std::map<std::string, std::unique_ptr<Window>> mWinsMap;
    static WindowsHandle* mHandle;
    static int mGlfwInited;
    std::string mWinInFocusName;

    Window* getWindowByName(std::string name);
    std::vector<char> mDbgBuffer;

    bool deleteWin(std::string name);
};
