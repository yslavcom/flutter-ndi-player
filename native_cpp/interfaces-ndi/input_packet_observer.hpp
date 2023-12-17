#pragma once

#include <stddef.h>

//#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Advanced.h>

#include <memory>

#include <functional>

class InputPacketsObserver
{
public:
    virtual void receivedVideoPack(std::unique_ptr<NDIlib_video_frame_v2_t> video, std::function<void(void* userData)>) = 0;
    virtual void receivedAudioPack(std::unique_ptr<NDIlib_audio_frame_v3_t> audio, void(*releaseCb)(void* ctxt, void* userData), void* context) = 0;
};
