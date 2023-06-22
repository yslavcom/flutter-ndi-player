#pragma once

#include "logger.hpp"

#include <queue>
#include <mutex>
#include <functional>

template <typename T>
class SafeQueue
{
public:
    SafeQueue(std::mutex& mu)
        : mMutex(mu)
        , mLIMIT(0)
    {}

    SafeQueue(std::mutex& mu, std::function<void(T*)> eraseElementCb, unsigned limitFrames)
        : mMutex(mu)
        , mEraseElementCb(eraseElementCb)
        , mLIMIT(limitFrames)
    {}

    virtual ~SafeQueue()
    {
        clearQueue();
    }

    bool push(T&& val)
    {
        std::lock_guard lock(mMutex);
        if (mLIMIT > 0 && getCountUnsafe() > mLIMIT)
        {
            LOGE("!!! Overflow\n");
            removeOldestElement();
        }

        mQueue.emplace(val);
        return true;
    }

    unsigned getCountUnsafe() const
    {
        return mQueue.size();
    }

    unsigned getCount() const
    {
        std::lock_guard lock(mMutex);
        return getCountUnsafe();
    }

    bool read(T& val)
    {
        std::lock_guard lock(mMutex);
        if (getCountUnsafe())
        {
            val = mQueue.front();
            mQueue.pop();
            return true;
        }
        return false;
    }

    void clearQueue()
    {
        std::lock_guard lock(mMutex);
        while (getCountUnsafe())
        {
            removeOldestElement();
        }
    }

protected:
    std::queue<T> mQueue;

private:
    std::mutex& mMutex;
    std::function<void(T*)> mEraseElementCb;

    void removeOldestElement()
    {
        T val = mQueue.front();
        mQueue.pop();
        if (mEraseElementCb)
        {
            mEraseElementCb(&val);
        }
    }

    const unsigned mLIMIT;
};
