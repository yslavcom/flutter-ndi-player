
#pragma once

#include <cstddef>

// #include <Processing.NDI.Advanced.h>
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Recv.h>

#include "interfaces-ndi/input_packet_observer.hpp"

#include <string>
#include <memory>
#include <set>
#include <functional>
#include <map>
#include <mutex>

class NdiApp
{
public:
    enum class Quality
    {
        Low,
        High
    };

    NdiApp();
    ~NdiApp();

    bool createReceiver(Quality quality);
    bool createReceiverUcompressed(Quality quality);
    bool connect(const std::string& name, const std::string& url);
    bool stopReceiver();
    void disconnectReceiver();
    bool capturePackets();

    void requestKeyFrame();

    void addObserver(InputPacketsObserver* obs);
    void removeObserver(InputPacketsObserver* obs);

private:
    class RecvClass
    {
    public:
        RecvClass(const NDIlib_recv_create_v3_t& arg)
        {
        /*
        1) There's a limit imposed by NDI lib how long the video frames are recieved in RX mode.
        It is set to 5 minutes.
        A valid vendor ID is required to unlock this limitation.
        Tested with NDI Advanced SDK.

        2) A valid vendor |ID is required to unlock the decompression of the SHQ2 frame (recvDescHi.color_format = NDIlib_recv_color_format_fastest)
        */

#if 0
            const char* ndiConfig = R"({
                "ndi": {
                    "rudp": { "recv": {"enable": true}},
                    "tcp": { "recv": {"enable": false}}
                }
            })";
#else
            #include "ndi-conf.hpp"
#endif


            mVal = NDIlib_recv_create_v3(&arg);///*NDIlib_recv_create_v4*/(&arg, ndiConfig);
        }
        RecvClass(const RecvClass&) = delete;
        RecvClass& operator=(const RecvClass&) = delete;
        RecvClass(RecvClass&& rhs)
        {
            mVal = rhs.mVal;
            rhs.mVal = nullptr;
        }
        RecvClass& operator=(RecvClass&& rhs)
        {
            mVal = rhs.mVal;
            rhs.mVal = nullptr;
            return *this;
        }
        ~RecvClass()
        {
            if (mVal)
            {
                NDIlib_recv_destroy(mVal);
            }
        }
        NDIlib_recv_instance_t operator()()
        {
            return mVal;
        }
        NDIlib_recv_instance_t src()
        {
            return mVal;
        }
        operator bool() const
        {
            return mVal != nullptr;
        }

    private:
        NDIlib_recv_instance_t mVal = nullptr;
    };

    std::shared_ptr<RecvClass> mRecvInst;

    bool captureBlock(std::shared_ptr<RecvClass> rxInst); // a blocking function
    std::map <NDIlib_audio_frame_v3_t*, std::shared_ptr<RecvClass>> mAudRevMap;

    bool handleVideo(std::unique_ptr<NDIlib_video_frame_v2_t>, std::shared_ptr<RecvClass> rxInst);
    bool handleAudio(std::unique_ptr<NDIlib_audio_frame_v3_t>, std::shared_ptr<RecvClass> rxInst);


    void releaseAudioSample(void* releaseData);
    static void releaseAudioSampleS(void* context, void* releaseData);

    template<typename T, typename C, typename... Args>
    void receivedPack(std::unique_ptr<T> pack, C cb, Args... args);
    std::set<InputPacketsObserver*> mInputPacketsObservers;

    bool mUncompressedShq;

    std::chrono::steady_clock::time_point mTimeRefr;

    std::mutex mMutex;

    std::chrono::steady_clock::time_point mLastVideoFrameTime;
    unsigned mDbgVidFrameCount = 0;
    unsigned mDbgPrevVidFrameCount = 0;
};