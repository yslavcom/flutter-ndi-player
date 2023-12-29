#pragma once

#include "safe-queue.hpp"
#include "logger.hpp"

#include <functional>
#include <variant>
#include <vector>

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

 enum class FrameFormatType
{
	progressive,
	interleaved,

	// Individual fields.
	field_0,
	field_1,

    unknown
};

struct VideoFrameCompressedStr
{
    void *opaque;

    int xres;
    int yres;
    unsigned fourCC;
	int frameRateN;
    int frameRateD;
	float aspectRatio;
	FrameFormatType frameFormatType;

	// The video data itself.
	uint8_t* p_data;
	size_t dataSizeBytes;

    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    bool isKeyFrame;
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
typedef void(*ReleaseCbAud)(void*, void*);

using VideoFrameVariant = std::variant<VideoFrameStr, VideoFrameCompressedStr>;
using VideoFrame = std::pair<VideoFrameVariant, ReleaseCb>;
using AudioFrame = std::pair<AudioFrameStr, std::pair<ReleaseCbAud, void*>>;

// helper type for the visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template < class F>
void release(VideoFrameVariant& var, F cleanup)
{
    std::visit( overloaded {
                        [cleanup](VideoFrameStr& arg){
                            cleanup(arg.opaque);
                        },
                        [cleanup](VideoFrameCompressedStr& arg){
                            cleanup(arg.opaque);
                        }
    }, var);
}

template < class F>
void release(AudioFrameStr& var, F cleanup)
{
    cleanup(var.opaque);
}

template <typename T>
class Queue_: public SafeQueue<T>
{
public:
    Queue_(std::mutex& mu)
        : SafeQueue<T>(mu
        , [](T* el)
        {
            (void)el;
#if 0
            if (el && el->second)
            {
                release(el->first, el->second);
            }
#endif
        }
        , 500)
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
