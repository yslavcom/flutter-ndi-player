#pragma once

#include <queue>
#include <mutex>
#include <functional>

template <typename T>
class SafeQueue
{
public:
    SafeQueue(std::mutex& mu)
        : mMutex(mu)
    {}

    SafeQueue(std::mutex& mu, std::function<void(T*)> eraseElementCb)
        : mMutex(mu)
        , mEraseElementCb(eraseElementCb)
    {}

    virtual ~SafeQueue()
    {
        clearQueue();
    }

    bool push(T&& val)
    {
        std::lock_guard lock(mMutex);
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
        if (!mEraseElementCb)
        {
        }
        else
        {
            while (getCountUnsafe())
            {
                T val = mQueue.front();
                mQueue.pop();
                mEraseElementCb(&val);
            }
        }
    }

protected:
    std::queue<T> mQueue;

private:
    std::mutex& mMutex;
    std::function<void(T*)> mEraseElementCb;
};
