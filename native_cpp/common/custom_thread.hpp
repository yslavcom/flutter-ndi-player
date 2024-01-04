#pragma once

#include "common/logger.hpp"

#include <thread>
#include <atomic>
#include <string>

#define _DBG_THREAD

#ifdef _DBG_THREAD
    #define DBG_THREAD LOGW
#else
    #define DBG_THREAD
#endif

class CustomThread final
{
public:
    ~CustomThread()
    {
        terminate();
    }

    template <typename Callback, typename... Args>
    bool start(std::string name, Callback cb, Args ...arg)
    {
        mDbgName = name;
        if (!mThread.joinable())
        {
            auto lambda = [this](Callback cb, Args... arg)
            {
                while(!mStop)
                {
                    cb(mStop, arg...);
                }
            };

            mThread = std::thread(lambda, cb, arg...);
        }

        return mThread.joinable();
    }

    bool terminate()
    {
        if (mThread.joinable())
        {
            DBG_THREAD("Thread terminate\n");
            mStop = true;
            mThread.join();
        }
        return !mThread.joinable();
    }

private:
    std::thread mThread;
    std::atomic<bool> mStop{false};

    std::string mDbgName;
};

