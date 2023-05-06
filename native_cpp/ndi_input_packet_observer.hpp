#pragma once

#include "interfaces-ndi/input_packet_observer.hpp"
#include "common/frame-queue.hpp"

#include <iostream>

class NdiInputPacketsObserver : public InputPacketsObserver
{
public:
    NdiInputPacketsObserver(FrameQueue::VideoRx& videoRxQueue, FrameQueue::AudioRx& audioRxQueue)
        : mVideoRxQueue(videoRxQueue)
        , mAudioRxQueue(audioRxQueue)
    {}

    void receivedVideoPack(NDIlib_video_frame_v2_t *video, std::function<void(void* userData)> releaseCb) override
    {
        if (!video)
        {
            return;
        }

        bool compressed = isCompressed(video->FourCC);

        if (compressed)
        {
            LOGE("compressed video not yet implemented, FourCC:%d\n", video->FourCC);
        }
        else
        {
            FrameQueue::VideoFrameStr frame;
            frame.opaque = (void*)video;
            frame.data = video->p_data;
            frame.fourCC = video->FourCC;
            frame.stride = video->line_stride_in_bytes;
            frame.xres = video->xres;
            frame.yres = video->yres;
            mVideoRxQueue.push(std::make_pair(frame, releaseCb));
        }
    }

    void receivedAudioPack(NDIlib_audio_frame_v3_t *audio, std::function<void(void* userData)> releaseCb) override
    {

        FrameQueue::AudioFrameStr frame;
        frame.opaque = (void*)audio;
        frame.chanNo = audio->no_channels;
        frame.samplesNo = audio->no_samples;
        frame.samples = audio->p_data;
        // TODO: check FOURCC here to figure out the right stride
        frame.stride = audio->channel_stride_in_bytes;
        mAudioRxQueue.push(std::make_pair(frame, releaseCb));
    }

private:
    FrameQueue::VideoRx& mVideoRxQueue;
    FrameQueue::AudioRx& mAudioRxQueue;

    bool isCompressed(NDIlib_FourCC_video_type_e type) const
    {
        switch(type)
        {
        case NDIlib_FourCC_type_UYVY:
        case NDIlib_FourCC_type_UYVA:
        case NDIlib_FourCC_type_P216:
        case NDIlib_FourCC_type_PA16:
        case NDIlib_FourCC_type_YV12:
        case NDIlib_FourCC_type_I420:

        case NDIlib_FourCC_type_NV12:
        case NDIlib_FourCC_type_BGRA:
        case NDIlib_FourCC_type_BGRX:
        case NDIlib_FourCC_type_RGBA:

        case NDIlib_FourCC_type_RGBX:
            return false;

        case NDIlib_FourCC_video_type_max:
            // not sure here
            return true;
        }
        return true;
    }
};
