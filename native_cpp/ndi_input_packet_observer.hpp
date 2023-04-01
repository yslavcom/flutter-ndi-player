#pragma once

#include "interfaces-ndi/input_packet_observer.hpp"

#include <iostream>

class NdiInputPacketsObserver : public InputPacketsObserver
{
public:
    void receivedVideoPack(NDIlib_video_frame_v2_t *video, std::function<void(void* userData)> releaseCb) override
    {
        LOGW("video:%d\n", video->xres);
    }

    void receivedAudioPack(NDIlib_audio_frame_v3_t *audio, std::function<void(void* userData)> releaseCb) override
    {
        LOGW("audio:%d\n", audio->no_samples);
    }
};
