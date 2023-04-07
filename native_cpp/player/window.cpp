#include "window.hpp"
#include "texture.hpp"
#include "common/conv-scale.hpp"

#include <iostream>
#include <cstring>
#include <memory>

#if 0

/**********************************************
 *
 *  Single Window
 *
*/
class Window
{
public:
    struct Dimensions
    {
        Dimensions()
            : xRes(1280), yRes(720)
        {}

        Dimensions(unsigned w, unsigned h)
            : xRes(w), yRes(h)
        {}

        Dimensions(const Dimensions& rhs)
        {
            xRes = rhs.xRes;
            yRes = rhs.yRes;
        }


        unsigned xRes;
        unsigned yRes;
    };

    Window(std::string name, unsigned xRes, unsigned yRes, OnCloseCallback onCloseCb);
    ~Window();
    bool setContext();
    bool setFocus();
    void swapBuffers();
    bool getWinFrameBufSize(int& w, int& h);
    bool loadTex(uint8_t* frameBuf);

    operator bool() const { return mWindow != nullptr; }
    bool valid() const { return mWindow != nullptr; }
    auto ptr() { return mWindow; }

    bool shouldClose();
    void runRender1();
    void runRender2();
    std::unique_ptr<uint8_t[]> convScaleFrame(const FrameQueue::VideoFrame& frame, size_t& size);
    void passFrame(const FrameQueue::VideoFrame& video);
    void renderFrame();
    std::string getName() const;
    bool setName(std::string newName);
    bool closeWindow();

private:
    GLFWwindow* mWindow;
    Dimensions mDimViewport;
    Dimensions mDimTex;
    std::string mWinName;
    std::unique_ptr<Texture2D> mTex;

    void framebuffer_size_callback(int width, int height);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void window_close_callback();
    static void window_close_callback(GLFWwindow* window);
    static std::map<GLFWwindow*, Window&> mWindowMap;

    std::unique_ptr<ConvertScale> mConvertScale;
    const FrameQueue::VideoFrame* mVideoFrame = nullptr;

    void render();

    OnCloseCallback mOnCloseCallback;
};

std::map<GLFWwindow*, Window&> Window::mWindowMap;

Window::Window(std::string name, unsigned xRes, unsigned yRes, OnCloseCallback onCloseCb)
    : mWinName(name)
    , mDimViewport(1280, 720)
    , mDimTex(xRes, yRes)
    , mOnCloseCallback(onCloseCb)
{
    mWindow = glfwCreateWindow(mDimTex.xRes, mDimTex.yRes, mWinName.c_str(), nullptr, nullptr);
    std::cout << __func__ << " " << (uintptr_t)mWindow << std::endl;
    if (!mWindow)
    {
        std::cerr << "Failed to create window:" << name << std::endl;
    }
    else
    {
        mTex = std::make_unique<Texture2D>();
        mWindowMap.insert({mWindow, *this});

        glViewport(0, 0, mDimViewport.xRes, mDimViewport.yRes);

        glfwSetFramebufferSizeCallback(mWindow, &Window::framebuffer_size_callback);
        glfwSetWindowCloseCallback(mWindow, &Window::window_close_callback);
    }
}

Window::~Window()
{
    std::cout << __func__ << " " << mWinName << std::endl;
    mTex.reset();
    auto it = mWindowMap.find(mWindow);
    if (it != mWindowMap.cend())
    {
        mWindowMap.erase(it);
    }
    glfwDestroyWindow(mWindow);
}

bool Window::setContext()
{
    glfwMakeContextCurrent(mWindow);
    return true;
}

bool Window::setFocus()
{
    glfwFocusWindow(mWindow);
    return true;
}

void Window::swapBuffers()
{
    glfwSwapBuffers(mWindow);
}

bool Window::getWinFrameBufSize(int& w, int& h)
{
    glfwGetFramebufferSize(mWindow, &w, &h);
    return true;
}

bool Window::loadTex(uint8_t* frameBuf)
{
    mTex->bind();
    auto ret = mTex->loadImage(0, GL_RGBA, mDimTex.xRes, mDimTex.yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, frameBuf);
    mTex->unbind();
    return ret;
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void Window::render()
{
    auto texW = mDimTex.xRes;
    auto texH = mDimTex.yRes;
    // Render
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2i(0, 0);
        glTexCoord2d(1, 0); glVertex2i(texW, 0);
        glTexCoord2d(1, 1); glVertex2i(texW, texH);
        glTexCoord2d(0, 1); glVertex2i(0, texH);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void Window::runRender1()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, mDimTex.xRes, mDimTex.yRes, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void Window::runRender2()
{
    render();
    swapBuffers();
}

void Window::framebuffer_size_callback(int width, int height)
{
    mDimViewport.xRes = width;
    mDimViewport.yRes = height;

    glViewport(0, 0, mDimViewport.xRes, mDimViewport.yRes);
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (window)
    {
        // get the Window object holding this GLFWwindow
        auto it = mWindowMap.find(window);
        if (it != mWindowMap.cend())
        {
            it->second.framebuffer_size_callback(width, height);
        }
    }
}

void Window::window_close_callback()
{
    std::cout << __func__ << std::endl;
    if (mOnCloseCallback)
    {
        mOnCloseCallback();
    }
}

void Window::window_close_callback(GLFWwindow* window)
{
    if (window)
    {
        // get the Window object holding this GLFWwindow
        auto it = mWindowMap.find(window);
        if (it != mWindowMap.cend())
        {
            it->second.window_close_callback();
        }
    }
}

std::unique_ptr<uint8_t[]> Window::convScaleFrame(const FrameQueue::VideoFrame& frame, size_t& size)
{
    const int outputPixelComponentscount = 4; //RGBA

    auto xRes = mDimTex.xRes;
    auto yRes = mDimTex.yRes;

    size = xRes * yRes * outputPixelComponentscount; // RGBA
    auto rgbaFrame = std::make_unique<uint8_t[]>(size);
    if (!rgbaFrame)
    {
        std::cerr << "Failed to allocate memory for rgba frame" << std::endl;
        return nullptr;
    }

    if (!mConvertScale)
    {
        mConvertScale = std::make_unique<ConvertScale>(frame.xres, frame.yres, xRes, yRes);
    }

    uint8_t* src[3] = {frame.data, nullptr, nullptr};
    int bytes_in_line = (0 == frame.stride) ? frame.xres : frame.stride;
    int srcLineSize[4] = {bytes_in_line, bytes_in_line, bytes_in_line, bytes_in_line};

    uint8_t* dest[4] = {rgbaFrame.get(), nullptr, nullptr, nullptr};
    auto lineSize = static_cast<int>(xRes) * outputPixelComponentscount;
    int destLineSize[4] = {lineSize, lineSize, lineSize, lineSize};

    if (0 < mConvertScale->scale(xRes, yRes, frame.xres, frame.yres, src, srcLineSize, dest, destLineSize))
    {
        return rgbaFrame;
    }
    return nullptr;
}

void Window::passFrame(const FrameQueue::VideoFrame& video)
{
    mVideoFrame = &video;
}

void Window::renderFrame()
{
    if (mVideoFrame)
    {
        size_t size = 0;
        auto framePtr = convScaleFrame(*mVideoFrame, size);
        if (framePtr)
        {
            runRender1();
            loadTex(framePtr.get());
            runRender2();
        }
    }
}

std::string Window::getName() const
{
    return mWinName;
}

bool Window::setName(std::string newName)
{
    mWinName = newName;
    glfwSetWindowTitle(mWindow, mWinName.c_str());
    return true;
}

bool Window::closeWindow()
{
    glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
    return true;
}

/**********************************************
 *
 *  Windows Handler
 *
*/
WindowsHandle* WindowsHandle::mHandle = 0;
int WindowsHandle::mGlfwInited = 0;

WindowsHandle* WindowsHandle::getInstance()
{
    if (!mHandle)
    {
        mHandle = new WindowsHandle();

        if (glfwInit())
        {
            mGlfwInited = true;
        }
        else
        {
            std::cerr << "Failed to init GLFW" << std::endl;
        }
    }
    return mHandle;
}

WindowsHandle::~WindowsHandle()
{
    // delete all existing windows
    for (auto& win: mWinsMap)
    {
        win.second.reset();
    }

    if (mGlfwInited)
    {
        glfwTerminate();
        mGlfwInited --;
    }
}

bool WindowsHandle::create(std::string name, unsigned xRes, unsigned yRes, OnCloseCallback onCloseCb)
{
    std::cout << __func__ << " name:" << name << std::endl;
    auto win = std::make_unique<Window>(name, xRes, yRes, onCloseCb);
    if (!win)
    {
        return false;
    }
    else if(!win->valid())
    {
        return false;
    }
    auto iter = mWinsMap.find(name);
    if (iter != mWinsMap.end())
    {
        std::cout << __func__ << " erasing:" << name << std::endl;
        mWinsMap.erase(iter);
    }

    mWinsMap[name] = std::move(win);
    return true;
}

bool WindowsHandle::rename(std::string oldName, std::string newName)
{
    auto win = getWindowByName(oldName);
    if (!win)
    {
        return false;
    }
    return win->setName(newName);
}

bool WindowsHandle::closeAllWindows()
{
#if 1
    for (auto& el : mWinsMap)
    {
        deleteWin(el.first);
    }
    return true;
#else
    for (auto& el : mWinsMap)
    {
        el.second.get()->closeWindow();
    }
    return true;
#endif
}

bool WindowsHandle::deleteWin(std::string name)
{
    std::cout << __func__ << std::endl;

    auto iter = mWinsMap.find(name);
    if (iter != mWinsMap.end())
    {
        mWinsMap.erase(iter);
        return true;
    }
    return false;
}

bool WindowsHandle::setContext(std::string name)
{
    auto win = getWindowByName(name);
    if (win)
    {
        mWinInFocusName = name;
        return win->setContext();
    }
    return false;
}

bool WindowsHandle::setFocus(std::string name)
{
    auto win = getWindowByName(name);
    if (win)
    {
        mWinInFocusName = name;
        return win->setFocus();
    }
    return false;
}

Window* WindowsHandle::getWindowByName(std::string name)
{
    auto iter = mWinsMap.find(name);
    if (iter != mWinsMap.end())
    {
        return iter->second.get();
    }
    return nullptr;
}

void WindowsHandle::dbgFilleBuffer()
{
    mDbgBuffer.resize(100*100*3);
    for(auto y = 0; y < 100; y ++)
    {
        for(auto x = 0; x < 100; x ++)
        {
            mDbgBuffer[y*100 + x*3] = 0;
            mDbgBuffer[y*100 + x*3 + 1] = 255;
            mDbgBuffer[y*100 + x*3 + 2] = 0;
        }
    }

    for(auto y = 0; y < 75; y ++)
    {
        for(auto x = 0; x < 75; x ++)
        {
            mDbgBuffer[y*100 + x*3] = 0;
            mDbgBuffer[y*100 + x*3 + 1] = 0;
            mDbgBuffer[y*100 + x*3 + 2] = 255;
        }
    }
}

void WindowsHandle::pollEvents()
{
    glfwPollEvents();
}

void WindowsHandle::renderFrame(const FrameQueue::VideoFrame& video, unsigned destWinIdx)
{
    // TODO: change to acessing the needed window based on input arguments
    for (auto& el : mWinsMap)
    {
        auto win = el.second.get();
        if (win)
        {
            win->passFrame(video);
            win->renderFrame();
        }
    }
}

bool WindowsHandle::shouldClose()
{
    bool shouldClose = true;
    for (auto& el : mWinsMap)
    {
        auto win = el.second.get();
        if (!el.second->shouldClose())
        {
            shouldClose = false;
            break;
        }
        else
        {
            deleteWin(el.first);
        }
    }

    return shouldClose;
}

#endif