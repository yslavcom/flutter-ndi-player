#pragma once

#include "interfaces-ndi/input_packet_observer.hpp"
#include "common/frame-queue.hpp"
#include "player/sps_pps_parser.hpp"

#include "common/logger.hpp"

#include <iostream>

#define _DBG_NDI_INP_OBS
#ifdef _DBG_NDI_INP_OBS
    #define DBG_NDI_INP_OBS(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_NDI_INP_OBS(format, ...)
#endif

class NdiInputPacketsObserver : public InputPacketsObserver
{
public:
    NdiInputPacketsObserver(FrameQueue::VideoRx& videoRxQueue, FrameQueue::AudioRx& audioRxQueue)
        : mVideoRxQueue(videoRxQueue)
        , mAudioRxQueue(audioRxQueue)
        , mIsFirstFrame(true)
    {}

    void receivedVideoPack(std::unique_ptr<NDIlib_video_frame_v2_t> video, std::function<void(void* userData)> releaseCb) override
    {
        if (!video)
        {
            return;
        }

        uint32_t fourCC = __builtin_bswap32(video->FourCC);

        auto optCompressed = isCompressed((NDIlib_FourCC_video_type_ex_e)video->FourCC);
        if (!optCompressed.has_value())
        {
            // Display wanring about unkwown state?
            return;
        }
        bool compressed = optCompressed.value();

        /* 53485132 -> SHQ2, 55595659 -> UYVY*/

        // DBG_NDI_INP_OBS("receivedVideoPack, 4cc:%lx, compressed:%d\n", fourCC, compressed);

        if (compressed)
        {
            // We should strip off the prepended header.
            uint32_t hdrSize = ((uint32_t)video->p_data[3] << 24);
            hdrSize |= ((uint32_t)video->p_data[2] << 16);
            hdrSize |= ((uint32_t)video->p_data[1] << 8);
            hdrSize |= ((uint32_t)video->p_data[0]);

            if (hdrSize >= (uint32_t)video->data_size_in_bytes)
            {
                DBG_NDI_INP_OBS("Empty payload\n");
                if (releaseCb)
                {
                    releaseCb(video.release());
                }
                return;
            }
            else
            {
                auto si = H26x::tryParseServiceInfo(video->p_data+4, video->data_size_in_bytes-4, hdrSize-4);
                std::vector<uint8_t> sps;
                std::vector<uint8_t> pps;

                bool pushFrames = true;
                if (mIsFirstFrame)
                {
                    pushFrames = false;
                    if (si.has_value())
                    {
                        if (si->sps.size() && si->pps.size())
                        {
                            mIsFirstFrame = false;
                            pushFrames = true;

                            LOGW("The 1st key frame\n");
                        }

                        if (si->sps.size())
                        {
                            sps = std::move(si->sps);
                        }
                        if (si->pps.size())
                        {
                            pps = std::move(si->pps);
                        }
                    }
                }

                if (!pushFrames)
                {
                    LOGW("Wait for the first key frame\n");
                }

                if (pushFrames)
                {
#if 0
                    auto p = video->p_data + hdrSize;
                    DBG_NDI_INP_OBS("%s: bytes: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
                        p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
#endif

                    FrameQueue::VideoFrameCompressedStr frame;

                    frame.sps = std::move(sps);
                    frame.pps = std::move(pps);

                    frame.xres = video->xres;
                    frame.yres = video->yres;
                    frame.fourCC = fourCC;
                    frame.frameRateN = video->frame_rate_N;
                    frame.frameRateD = video->frame_rate_D;
                    frame.aspectRatio = video->picture_aspect_ratio;
                    frame.p_data = video->p_data + hdrSize + frame.sps.size() + frame.pps.size();
                    frame.dataSizeBytes = video->data_size_in_bytes - hdrSize - (frame.sps.size() + frame.pps.size());

                    switch(video->frame_format_type)
                    {
                    case NDIlib_frame_format_type_progressive:
                        frame.frameFormatType = FrameQueue::FrameFormatType::progressive;
                    break;

                    case NDIlib_frame_format_type_interleaved:
                        frame.frameFormatType = FrameQueue::FrameFormatType::interleaved;
                    break;

                    case NDIlib_frame_format_type_field_0:
                        frame.frameFormatType = FrameQueue::FrameFormatType::field_0;
                    break;

                    case NDIlib_frame_format_type_field_1:
                        frame.frameFormatType = FrameQueue::FrameFormatType::field_1;
                    break;

                    case NDIlib_frame_format_type_max:
                        frame.frameFormatType = FrameQueue::FrameFormatType::unknown;
                    break;
                    }

                    frame.opaque = (void*)video.release();

                    mVideoRxQueue.push(std::make_pair(frame, releaseCb));
                }
            }
        }
        else
        {
            FrameQueue::VideoFrameStr frame;
            frame.data = video->p_data;
            frame.fourCC = fourCC;
            frame.stride = video->line_stride_in_bytes;
            frame.xres = video->xres;
            frame.yres = video->yres;
            frame.opaque = (void*)video.release();

            mVideoRxQueue.push(std::make_pair(frame, releaseCb));
        }
    }

    void receivedAudioPack(std::unique_ptr<NDIlib_audio_frame_v3_t> audio, FrameQueue::ReleaseCbAud releaseCb, void* context) override
    {

        FrameQueue::AudioFrameStr frame;
        frame.chanNo = audio->no_channels;
        frame.samplesNo = audio->no_samples;
        frame.samples = audio->p_data;
        // TODO: check FOURCC here to figure out the right stride
        frame.stride = audio->channel_stride_in_bytes;
        frame.opaque = (void*)audio.release();

        mAudioRxQueue.push(std::make_pair(frame, std::make_pair(releaseCb, context)));
    }

private:
    FrameQueue::VideoRx& mVideoRxQueue;
    FrameQueue::AudioRx& mAudioRxQueue;

    std::optional<bool> isCompressed(NDIlib_FourCC_video_type_ex_e type) const
    {
        switch(type)
        {
    	// SpeedHQ formats at the highest bandwidth.
    	case NDIlib_FourCC_type_SHQ0_highest_bandwidth:	// Backwards compatibility
        case NDIlib_FourCC_type_SHQ2_highest_bandwidth:	// Backwards compatibility
        case NDIlib_FourCC_type_SHQ7_highest_bandwidth:	// Backwards compatibility
            return true;

    	// SpeedHQ formats at the lowest bandwidth.
        case NDIlib_FourCC_type_SHQ0_lowest_bandwidth:	// Backwards compatibility
        case NDIlib_FourCC_type_SHQ2_lowest_bandwidth:	// Backwards compatibility
        case NDIlib_FourCC_type_SHQ7_lowest_bandwidth:	// Backwards compatibility
            return true;

    	// If SpeedHQ 4:4:4 / 4:4:4:4 formats are desired, please contact ndi@newtek.com.

    	// H.264 video at the highest bandwidth -- the data field is expected to be prefixed with the
    	// NDIlib_compressed_packet_t structure.
        case NDIlib_FourCC_type_H264_highest_bandwidth:	// Backwards compatibility

    	// H.264 video at the lowest bandwidth -- the data field is expected to be prefixed with the
    	// NDIlib_compressed_packet_t structure.
        case NDIlib_FourCC_type_H264_lowest_bandwidth:	// Backwards compatibility

    	// H.265/HEVC video at the highest bandwidth -- the data field is expected to be prefixed with
    	// the NDIlib_compressed_packet_t structure.
        case NDIlib_FourCC_type_HEVC_highest_bandwidth:	// Backwards compatibility

    	// H.265/HEVC video at the lowest bandwidth -- the data field is expected to be prefixed with
    	// the NDIlib_compressed_packet_t structure.
        case NDIlib_FourCC_type_HEVC_lowest_bandwidth:	// Backwards compatibility

    	// H.264 video at the highest bandwidth -- the data field is expected to be prefixed with the
    	// NDIlib_compressed_packet_t structure.
    	//
    	// This version is basically a frame of double the height where to top part is the color and the bottom
    	// half has the alpha channel (against chroma being gray).
        case NDIlib_FourCC_type_h264_alpha_highest_bandwidth:	// Backwards compatibility

    	// H.264 video at the lowest bandwidth -- the data field is expected to be prefixed with the
    	// NDIlib_compressed_packet_t structure.
    	//
    	// This version is basically a frame of double the height where to top part is the color and the bottom
    	// half has the alpha channel (against chroma being gray).
        case NDIlib_FourCC_type_h264_alpha_lowest_bandwidth:	// Backwards compatibility

    	// H.265/HEVC video at the highest bandwidth -- the data field is expected to be prefixed with
    	// the NDIlib_compressed_packet_t structure.
    	//
    	// This version is basically a frame of double the height where to top part is the color and the bottom
    	// half has the alpha channel (against chroma being gray).
        case NDIlib_FourCC_type_HEVC_alpha_highest_bandwidth:	// Backwards compatibility

    	// H.265/HEVC video at the lowest bandwidth -- the data field is expected to be prefixed with
    	// the NDIlib_compressed_packet_t structure.
    	//
    	// This version is basically a frame of double the height where to top part is the color and the bottom
    	// half has the alpha channel (against chroma being gray).
        case NDIlib_FourCC_type_HEVC_alpha_lowest_bandwidth:	// Backwards compatibility
            return true;

    	// Make sure this is a 32-bit enumeration.
    	case NDIlib_FourCC_video_type_ex_max:
            // not sure here
            break;
        }

        switch((NDIlib_FourCC_video_type_e)type)
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
            break;
        }

        return {};
    }

    bool mIsFirstFrame;
};
