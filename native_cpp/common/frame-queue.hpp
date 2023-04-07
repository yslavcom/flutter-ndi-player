#pragma once

#include "safe-queue.hpp"
#include <functional>

namespace FrameQueue
{
struct VideoFrame
{
    void *opaque;
    uint8_t* data;
    int xres;
    int yres;
    int stride;
    unsigned fourCC;
};

struct AudioFrame
{
    void *opaque;
    unsigned chanNo;
    uint8_t* samples;
    unsigned samplesNo;
    unsigned stride;
    bool planar;
};

using ReleaseCb = std::function<void(void*)>;
using Frame = std::pair<VideoFrame, ReleaseCb>;
using AudFrame = std::pair<AudioFrame, ReleaseCb>;

template <typename T>
class Queue_: public SafeQueue<T>
{
public:
    Queue_(std::mutex& mu)
        : SafeQueue<T>(mu)
        , mMutex(mu)
    {}

    virtual ~Queue_()
    {
#if 0
        flush();
#endif
    }

    void flush()
    {
#if 0
        std::lock_guard lock(mMutex);
        unsigned count = getCountUnsafe();
        while(count --)
        {
            auto val = mQueue.front();
            if (val.second)
            {
                val.second(val.first.opaque);
            }

            mQueue.pop();
        }
#endif
    }

private:
    std::mutex& mMutex;
};

using Queue = Queue_<Frame>;
using Audio = Queue_<AudFrame>;
}