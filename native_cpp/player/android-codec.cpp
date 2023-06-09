#include "android-codec.hpp"
#include "common/logger.hpp"

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>

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
    : mXRes(0)
    , mYRes(0)
    , mCodec(nullptr)
    , mCsdDataSps("csd_0")
    , mCsdDataPps("csd_1")
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

bool AndroidDecoder::create()
{
    mCodec = AMediaCodec_createDecoderByType(mH264Type);
    LOGW("mCodec:%p\n", mCodec);
    if (!mCodec) return false;

    mFormat = AMediaFormat_new();
    LOGW("mFormat:%p\n", mFormat);
    if (!mFormat) return false;

    return true;
}

bool AndroidDecoder::configure()
{
    DBG_ANDRDEC("AndroidDecoder::configure, isValid:%d, mH264Type:%s, mXRes:%d, mYRes:%d\n",
        isValid(), mH264Type, mXRes, mYRes);

    if (!isValid()) return false;

    AMediaFormat_setString(mFormat, AMEDIAFORMAT_KEY_MIME, mH264Type);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_WIDTH, mXRes);
    AMediaFormat_setInt32(mFormat, AMEDIAFORMAT_KEY_HEIGHT, mYRes);

#if 0
    // not sure about csdName in AMediaFormat_setBuffer
    AMediaFormat_setBuffer(mFormat, mCsdDataSps.name.c_str(), mCsdDataSps.data.data(), mCsdDataSps.data.size()); // Optional codec-specific data
    AMediaFormat_setBuffer(mFormat, mCsdDataPps.name.c_str(), mCsdDataPps.data.data(), mCsdDataPps.data.size()); // Optional codec-specific data
#endif


    DBG_ANDRDEC("AMediaCodec_configure:%p, %p, %p\n", mCodec, mFormat, mNativeWindow);
    media_status_t ret = AMediaCodec_configure(mCodec, mFormat, mNativeWindow, nullptr, 0);
    DBG_ANDRDEC("AMediaCodec_configure result:%d\n", ret);
    mIsReady = (ret == AMEDIA_OK);

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
    DBG_ANDRDEC("Enqueue frame:%p, %d\n", frameBuf, frameSize);
    auto timeoutUs = 40000;
    auto presentationTimeUs = 40000;
    // Submit input data to codec
    auto inputIndex = AMediaCodec_dequeueInputBuffer(mCodec, timeoutUs);
    if (inputIndex >= 0)
    {
        AMediaCodecBufferInfo bufferInfo;
        size_t bufferSize = 0;
        uint8_t* inputBuffer = AMediaCodec_getInputBuffer(mCodec, inputIndex, &bufferSize);
        // Copy the H.264 frame to the input buffer
        memcpy(inputBuffer, frameBuf, std::min(bufferSize, frameSize));
        AMediaCodec_queueInputBuffer(mCodec, inputIndex, 0, frameSize, presentationTimeUs, 0);
    }

    return true;
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
    }

    return true;
}

void AndroidDecoder::init(unsigned xRes, unsigned yRes, void* nativeWindow)
{
    if (!mIsDecoderLoop)
    {
        DBG_ANDRDEC("AndroidDecoder::init:%d, %d, %p\n", xRes, yRes, nativeWindow);
        mXRes = xRes;
        mYRes = yRes;
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
