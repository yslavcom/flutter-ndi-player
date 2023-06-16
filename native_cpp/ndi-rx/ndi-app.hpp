
#pragma once

#include <cstddef>

#include <Processing.NDI.Advanced.h>

#include "interfaces-ndi/input_packet_observer.hpp"

#include <string>
#include <memory>
#include <set>
#include <functional>

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

    bool createReceiver(const std::string& name, const std::string& url, Quality quality);
    bool capturePackets();

    void addObserver(InputPacketsObserver* obs);
    void removeObserver(InputPacketsObserver* obs);

private:
    class RecvClass
    {
    public:
        RecvClass(const NDIlib_recv_create_v3_t& arg)
        {
#if 1
            const char* ndiConfig = nullptr;
#else
            const char* ndiConfig = R"({
                "ndi": {
                    "rudp": { "recv": {"enable": true}},
                    "tcp": { "recv": {"enable": false}}
                }
            })";
#endif

//            mVal = NDIlib_recv_create_v4(&arg, ndiConfig);
            mVal = NDIlib_recv_create_v3(&arg);
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

    template<typename T>
    void receivedPack(std::unique_ptr<T> pack, std::function<void(void* userData)>);
    std::set<InputPacketsObserver*> mInputPacketsObservers;
};