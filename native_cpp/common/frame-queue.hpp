#pragma once

#include "safe-queue.hpp"
#include <functional>

namespace FrameQueue
{
struct VideoFrameStr
{
    void *opaque;
    uint8_t* data;
    int xres;
    int yres;
    int stride;
    unsigned fourCC;
};

struct AudioFrameStr
{
    void *opaque;
    unsigned chanNo;
    uint8_t* samples;
    unsigned samplesNo;
    unsigned stride;
    bool planar;
};

using ReleaseCb = std::function<void(void*)>;
using VideoFrame = std::pair<VideoFrameStr, ReleaseCb>;
using AudioFrame = std::pair<AudioFrameStr, ReleaseCb>;

template <typename T>
class Queue_: public SafeQueue<T>
{
public:
    Queue_(std::mutex& mu)
        : SafeQueue<T>(mu
        , [](T* el)
        {
            if (el)
            {
                if (el->second)
                {
                    el->second(el->first.opaque);
                }
            }
        }
        , 100)
    {
    }

    virtual ~Queue_()
    {
    }

    void flush()
    {
        this->clearQueue();
    }

private:
};

using VideoRx = Queue_<VideoFrame>;
using AudioRx = Queue_<AudioFrame>;
} // namespace FrameQueue