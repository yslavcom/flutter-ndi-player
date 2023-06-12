#include "android-codec.hpp"
#include "common/logger.hpp"

#include <media/NdkMediaFormat.h>

#include <chrono>

#define _DBG_ANDRDEC
#define _DBG_ANDRDEC_ERR

#ifdef _DBG_ANDRDEC
    #define DBG_ANDRDEC(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_ANDRDEC(format, ...)
#endif

#ifdef _DBG_ANDRDEC_ERR
    #define DBG_ANDRDEC_ERR(format, ...) LOGE(format, ## __VA_ARGS__)
#else
    #define DBG_ANDRDEC_ERR(format, ...)
#endif


AndroidDecoder::AndroidDecoder(RequestSetupCb cb)
    : mCodec(nullptr)
    , mIsStarted(false)
    , mNativeWindow(nullptr)
    , mIsDecoderLoop(false)
    , mIsValid(false)
    , mIsReady(false)
    , mDecoderLoop(nullptr)
    , mRequestSetupCb(cb)
{}

AndroidDecoder::~AndroidDecoder()
{
    if (mCodec)
    {
        if (mIsStarted)
        {
            stop();
        }
        AMediaCodec_delete(mCodec);
    }
    if (mFormat)
    {
        AMediaFormat_delete(mFormat);
    }
}

const char* AndroidDecoder::getFormatPresentation() const
{
    if (mFormat)
    {
        return AMediaFormat_toString(mFormat);
    }
    return nullptr;
}

void AndroidDecoder::onAsyncInputAvailable(AMediaCodec *codec, void *userdata, int32_t index)
{
    assert(codec);
    assert(userdata);
    reinterpret_cast<AndroidDecoder*>(userdata)->onAsyncInputAvailable(codec, index);
}

void AndroidDecoder::onAsyncInputAvailable(AMediaCodec *codec, int32_t index)
{
    assert(mCodec == codec);

    LOGW("OnAsyncInputAvailable, index:%d\n", index);
    mInputAvailableBufferIdx.emplace(index);
}

void AndroidDecoder::onAsyncOutputAvailable(AMediaCodec *codec, void *userdata, int32_t index, AMediaCodecBufferInfo *bufferInfo)
{
    assert(codec);
    assert(userdata);
    reinterpret_cast<AndroidDecoder*>(userdata)->onAsyncOutputAvailable(codec, index, bufferInfo);
}

void AndroidDecoder::onAsyncOutputAvailable(AMediaCodec *codec, int32_t index, AMediaCodecBufferInfo *bufferInfo)
{
    assert(mCodec == codec);

    DBG_ANDRDEC("OnAsyncOutputAvailable, index:%d, offset:%d, size:%d, presentationTimeUs:%d, flags:%d\n",
        index,
        bufferInfo->offset, bufferInfo->size, bufferInfo->presentationTimeUs, bufferInfo->flags);

    // ?????
    // Release the output buffer
    bool renderBuffer = true;
    AMediaCodec_releaseOutputBuffer(mCodec, index, renderBuffer);
}

void AndroidDecoder::onAsyncFormatChanged(AMediaCodec *codec, void *userdata, AMediaFormat *format)
{
    assert(codec);
    assert(userdata);
    reinterpret_cast<AndroidDecoder*>(userdata)->onAsyncFormatChanged(codec, format);
}

void AndroidDecoder::onAsyncFormatChanged(AMediaCodec *codec, AMediaFormat *format)
{
    assert(mCodec == codec);

    LOGW("OnAsyncFormatChanged:%s\n", AMediaFormat_toString(format));

    const char *outMime;
    int32_t w = 0, h = 0;
    AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &outMime);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &w);
    AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &h);

    LOGW("AMEDIAFORMAT_KEY_MIME:%s\n", outMime);
    LOGW("AMEDIAFORMAT_KEY_WIDTH:%d\n", w);
    LOGW("AMEDIAFORMAT_KEY_HEIGHT:%d\n", h);
}

void AndroidDecoder::onAsyncError(AMediaCodec *codec, void *userdata, media_status_t error, int32_t actionCode, const char *detail)
{
    assert(codec);
    assert(userdata);
    reinterpret_cast<AndroidDecoder*>(userdata)->onAsyncError(codec, error, actionCode, detail);
}

void AndroidDecoder::onAsyncError(AMediaCodec *codec, media_status_t error, int32_t actionCode, const char *detail)
{
    assert(mCodec == codec);

    LOGW("OnAsyncError, error:%d, actionCode:%d, detail:%s\n", error, actionCode, detail);
}

bool AndroidDecoder::create(uint32_t fourcc)
{
    H26x::FourCC fourCC(fourcc);
    if (fourCC != "H264")
    {
        LOGE("Unknown codec 4cc:%lx\n", fourcc);
        assert(0 && "Unknown codec 4cc");
        return false;
    }

    mCodec = AMediaCodec_createDecoderByType(mH264Type);
    LOGW("mCodec:%p\n", mCodec);
    if (!mCodec) return false;

#if 0
    AMediaCodecOnAsyncNotifyCallback callback{};
    callback.onAsyncInputAvailable = &AndroidDecoder::onAsyncInputAvailable;
    callback.onAsyncOutputAvailable = &AndroidDecoder::onAsyncOutputAvailable;
    callback.onAsyncFormatChanged = &AndroidDecoder::onAsyncFormatChanged;
    callback.onAsyncError = &AndroidDecoder::onAsyncError;
    AMediaCodec_setAsyncNotifyCallback(mCodec, callback, this);
#endif
    mFormat = AMediaFormat_new();
    LOGW("mFormat:%p\n", mFormat);
    if (!mFormat) return false;

    return true;
}

void AndroidDecoder::setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps)
{
    if (sps.size())
    {
        AMediaFormat_setBuffer(mFormat, "csd_0", sps.data(), sps.size());
    }

    if (pps.size())
    {
        AMediaFormat_setBuffer(mFormat, "csd_1", pps.data(), pps.size());
    }
}

bool AndroidDecoder::configure()
{
    DBG_ANDRDEC("AndroidDecoder::configure, isValid:%d, mH264Type:%s, mXRes:%d, mYRes:%d\n",
        isValid(), mH264Type, mXres, mYres);

    if (!isValid()) return false;

    AMediaFormat_setString(mFormat, AMEDIAFORMAT_KEY_MIME, mH264Type);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_WIDTH, mXres);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_HEIGHT, mYres);

#if 0
    // not sure about csdName in AMediaFormat_setBuffer
    AMediaFormat_setBuffer(mFormat, mCsdDataSps.name.c_str(), mCsdDataSps.data.data(), mCsdDataSps.data.size()); // Optional codec-specific data
    AMediaFormat_setBuffer(mFormat, mCsdDataPps.name.c_str(), mCsdDataPps.data.data(), mCsdDataPps.data.size()); // Optional codec-specific data
#endif


    DBG_ANDRDEC("AMediaCodec_configure:%p, %p, %p\n", mCodec, mFormat, mNativeWindow);
    media_status_t ret = AMediaCodec_configure(mCodec, mFormat, mNativeWindow, nullptr, 0);
    DBG_ANDRDEC("AMediaCodec_configure result:%d\n", ret);
    mIsReady = (ret == AMEDIA_OK);

    if (mIsReady)
    {
        mDecoderLoop->run();
    }

    return mIsReady;
}

bool AndroidDecoder::start()
{

    DBG_ANDRDEC("Decoder start\n");
    media_status_t ret = AMediaCodec_start(mCodec);
    mIsStarted = ret == AMEDIA_OK;
    return mIsStarted;
}

bool AndroidDecoder::stop()
{
    DBG_ANDRDEC("Decoder stop\n");
    media_status_t ret = AMediaCodec_stop(mCodec);
    mIsStarted = false; // regardless of the code returned, clear the started flag
    return ret == AMEDIA_OK;
}

bool AndroidDecoder::enqueueFrame(const uint8_t* frameBuf, size_t frameSize)
{
    std::chrono::microseconds us = std::chrono::duration_cast< std::chrono::microseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );
#if 1
    if (mIsStarted)
    {
        auto timeoutUs = 40000;
        auto presentationTimeUs = us.count();
        // Submit input data to codec
        auto inputIndex = AMediaCodec_dequeueInputBuffer(mCodec, timeoutUs);
        if (inputIndex >= 0)
        {
            AMediaCodecBufferInfo bufferInfo;
            size_t bufferSize = 0;
            uint8_t* inputBuffer = AMediaCodec_getInputBuffer(mCodec, inputIndex, &bufferSize);
            if (inputBuffer)
            {
                // Copy the H.264 frame to the input buffer
                memcpy(inputBuffer, frameBuf, std::min(bufferSize, frameSize));
                DBG_ANDRDEC("Enqueue input buffer, codec:%p, idx:%d, size:%d, presentationTimeUs:%d\n", mCodec, inputIndex, frameSize, presentationTimeUs);
                media_status_t ret = AMediaCodec_queueInputBuffer(mCodec, inputIndex, 0, frameSize, presentationTimeUs, 0);
                return ret == AMEDIA_OK;
            }
        }
    }
    return false;

#else
    if (mIsStarted)
    {
        //auto presentationTimeUs = 40000;
        uint64_t presentationTimeUs = us.count();
        // Submit input data to codec

        if (mInputAvailableBufferIdx.size())
        {
            auto inputIndex = mInputAvailableBufferIdx.front();
            mInputAvailableBufferIdx.pop();

            AMediaCodecBufferInfo bufferInfo;
            size_t bufferSize = 0;
            uint8_t* inputBuffer = AMediaCodec_getInputBuffer(mCodec, inputIndex, &bufferSize);
            if (inputBuffer)
            {
                // Copy the H.264 frame to the input buffer
                memcpy(inputBuffer, frameBuf, std::min(bufferSize, frameSize));
                DBG_ANDRDEC("Enqueue input buffer, codec:%p, idx:%d, size:%d, presentationTimeUs:%d\n", mCodec, inputIndex, frameSize, presentationTimeUs);
                media_status_t ret = AMediaCodec_queueInputBuffer(mCodec, inputIndex, 0, frameSize, presentationTimeUs, 0);
                return ret == AMEDIA_OK;
            }
        }
    }

    return false;
#endif

}

bool AndroidDecoder::retrieveFrame()
{
    DBG_ANDRDEC("Retrieve frame\n");

    // Retrieve decoded output frames
    AMediaCodecBufferInfo bufferInfo{};
    auto timeoutUs = 40000;
    auto outputIndex = AMediaCodec_dequeueOutputBuffer(mCodec, &bufferInfo, timeoutUs);
    if (outputIndex >= 0)
    {
        size_t bufferSize;
        uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(mCodec, outputIndex, &bufferSize);
        // Process the decoded output frame in outputBuffer
        // ...

        // Release the output buffer
        bool renderBuffer = true;
        AMediaCodec_releaseOutputBuffer(mCodec, outputIndex, renderBuffer);

        DBG_ANDRDEC("retrieveFrame, idx:%d, size:%d\n", outputIndex, bufferSize);
    }

    return true;
}

void AndroidDecoder::init(void* nativeWindow)
{
    if (!mIsDecoderLoop)
    {
        DBG_ANDRDEC("AndroidDecoder::init:%p\n", nativeWindow);
        mNativeWindow = reinterpret_cast<ANativeWindow *>(nativeWindow);

        mDecoderLoop.reset(new DecoderLoop(this, mVidFramesToDecode, mDecodedVideoFrames));

        DBG_ANDRDEC("AndroidDecoder::init, mDecoderLoop:%p\n", mDecoderLoop.get());

        mIsValid = true;

        mIsDecoderLoop = true;
    }
}

void AndroidDecoder::requestSetup()
{
    DBG_ANDRDEC("AndroidDecoder::requestSetup set: %d\n", mRequestSetupCb ? true : false);
    if (mRequestSetupCb)
    {
        mRequestSetupCb(this);
    }
    else
    {
        DBG_ANDRDEC_ERR("Setup request not assigned !");
    }
}

bool AndroidDecoder::isReady() const
{
    return mIsReady;
}

bool AndroidDecoder::isValid() const
{
    return mIsValid;
}
