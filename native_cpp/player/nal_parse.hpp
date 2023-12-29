#pragma once

#include "common/frame-queue.hpp"
#include "sps_pps_parser.hpp"

#include <memory>
#include <optional>

#if 0
class NalParse
{
public:
    NalParse(H26x::FourCC fourcc);
    ~NalParse() = default;

    bool ppsSps() const;
    void parsePpsSps(const FrameQueue::VideoFrameCompressedStr& frm);
    bool keyFrame(const FrameQueue::VideoFrameCompressedStr& frm) const;

    class StreamSpsPps
    {
    public:
        StreamSpsPps() : mServiceInfo{}
        {}

        virtual ~StreamSpsPps(){}

        virtual bool ppsSps() const
        {
            if (mServiceInfo)
            {
                return (mServiceInfo->sps.size() > 0) && (mServiceInfo->pps.size() > 0);
            }
            return false;
        }

        virtual void parsePpsSps(const FrameQueue::VideoFrameCompressedStr& frm) = 0;
        virtual bool keyFrame(const FrameQueue::VideoFrameCompressedStr& frm) const = 0;

    protected:
        std::optional<H26x::ServiceInfo> mServiceInfo;
    };

private:
    std::unique_ptr<StreamSpsPps> mStreamSpsPps;
};
#endif // #if 0