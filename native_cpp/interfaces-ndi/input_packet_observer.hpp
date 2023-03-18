#pragma once

#include </home/iaroslav/work/streaming/NDI/5.5.3/NDI-Advanced-SDK-for-Linux/include/Processing.NDI.Advanced.h>

#include <functional>

class InputPacketsObserver
{
public:
    virtual void receivedVideoPack(NDIlib_video_frame_v2_t *video, std::function<void(void* userData)>) = 0;
    virtual void receivedAudioPack(NDIlib_audio_frame_v3_t *audio, std::function<void(void* userData)>) = 0;
};
