#include "nal_parse.hpp"

#if 0
class H264SpsPps: public NalParse::StreamSpsPps
{
public:
    H264SpsPps() = default;
    virtual ~H264SpsPps(){}

    virtual void parsePpsSps(const FrameQueue::VideoFrameCompressedStr& frm) override
    {
        const uint8_t * data = 0;
        size_t sz = 0;
        size_t hdrSz = 0;
        auto info = H26x::tryParseServiceInfo(H26x::FourCcType::H264, data, sz, hdrSz);
        if (info)
        {
            mServiceInfo = info;
        }
    }

    virtual bool keyFrame(const FrameQueue::VideoFrameCompressedStr& frm) const override
    {
        return false;
    }
};

class HevcSpsPps: public NalParse::StreamSpsPps
{
public:
    HevcSpsPps() = default;
    virtual ~HevcSpsPps(){}

    virtual void parsePpsSps(const FrameQueue::VideoFrameCompressedStr& frm) override
    {}

    virtual bool keyFrame(const FrameQueue::VideoFrameCompressedStr& frm) const override
    {
        return false;
    }
};

NalParse::NalParse(H26x::FourCC fourcc)
{
    if (fourcc.isH264())
    {
        mStreamSpsPps.reset(new H264SpsPps());
    }
    else if(fourcc.isHevc())
    {
        mStreamSpsPps.reset(new HevcSpsPps());
    }
}

bool NalParse::ppsSps() const
{
    if (mStreamSpsPps)
    {
        return mStreamSpsPps->ppsSps();
    }
    return false;
}

void NalParse::parsePpsSps(const FrameQueue::VideoFrameCompressedStr& frm)
{
    if (mStreamSpsPps)
    {
        mStreamSpsPps->parsePpsSps(frm);
    }
}

bool NalParse::keyFrame(const FrameQueue::VideoFrameCompressedStr& frm) const
{
    if (mStreamSpsPps)
    {
        return mStreamSpsPps->keyFrame(frm);
    }
}
#endif // #if 0
