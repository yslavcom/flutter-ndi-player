#include "conv-scale.hpp"

#include "common/logger.hpp"

extern "C" {
#include "libswscale/swscale.h"
}

#include <iostream>

struct ScaleContext
{
    ScaleContext()
        : mSwsContext(nullptr)
    {}

    ~ScaleContext()
    {
        if (mSwsContext)
        {
            sws_freeContext(mSwsContext);
        }
    }

    SwsContext* mSwsContext;
};

ConvertScale::ConvertScale()
{
    mScaleContext = new ScaleContext;
}

ConvertScale::ConvertScale(unsigned srcW, unsigned srcH, unsigned dstW, unsigned dstH)
        : ConvertScale()
{
    mScaleContext->mSwsContext = sws_getCachedContext(mScaleContext->mSwsContext, srcW, srcH, AV_PIX_FMT_UYVY422,
                                dstW, dstH, AV_PIX_FMT_RGB0,
                                SWS_FAST_BILINEAR,
                                nullptr, nullptr, nullptr);
    if (!mScaleContext->mSwsContext)
    {
        LOGE("Failed to init sw scaler\n");
    }
}

ConvertScale::~ConvertScale()
{
    if (mScaleContext)
    {
        delete (mScaleContext);
    }
}

int ConvertScale::scale(unsigned srcH, const uint8_t *const srcSlice[], const int srcStride[], uint8_t *const dst[], const int dstStride[])
{
    int ret = sws_scale(mScaleContext->mSwsContext,
        srcSlice, srcStride, 0, srcH,
        dst, dstStride);
    if (!(ret > 0))
    {
        LOGE("ret:%d", ret);
    }
    return ret;
}

int ConvertScale::scale(unsigned dstW, unsigned dstH, unsigned srcW, unsigned srcH, const uint8_t *const srcSlice[], const int srcStride[], uint8_t *const dst[], const int dstStride[])
{
    mScaleContext->mSwsContext = sws_getCachedContext(mScaleContext->mSwsContext, srcW, srcH, AV_PIX_FMT_UYVY422,
                                dstW, dstH, AV_PIX_FMT_RGB0,
                                SWS_FAST_BILINEAR,
                                nullptr, nullptr, nullptr);

    return sws_scale(mScaleContext->mSwsContext,
        srcSlice, srcStride, 0, srcH,
        dst, dstStride);
}
