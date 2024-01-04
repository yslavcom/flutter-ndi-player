#pragma once

#include "common/logger.hpp"

#include <thread>
#include <atomic>
#include <string>

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

