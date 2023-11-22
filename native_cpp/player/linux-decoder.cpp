#ifdef LINUX_PLATFORM

#include "linux-decoder.hpp"

#define _DBG_LINDECODER

#ifdef _DBG_LINDECODER
    #define DBG_LINDECODER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_LINDECODER(format, ...)
#endif

LinuxDecoder::LinuxDecoder(RequestSetupCb cb)
    : mIsStarted(false)
    , mIsReady(false)
{
    DBG_LINDECODER("LinuxDecoder\n");
}

LinuxDecoder::~LinuxDecoder()
{
    DBG_LINDECODER("\n");
}

bool LinuxDecoder::create(uint32_t fourcc)
{
    DBG_LINDECODER("\n");
    return false;
}

bool LinuxDecoder::configure()
{
    DBG_LINDECODER("\n");

    if (!mDecoderLoop)
    {
        return false;
    }

    mIsReady = true;
    if (mIsReady)
    {
        mDecoderLoop->run();
    }

    return mIsReady;
}

bool LinuxDecoder::start()
{
    DBG_LINDECODER("\n");

    mIsStarted = true;
    return true;
}

bool LinuxDecoder::stop()
{
    DBG_LINDECODER("\n");

    mIsStarted = false;
    return true;
}

void LinuxDecoder::release()
{
    DBG_LINDECODER("\n");
}

bool LinuxDecoder::enqueueFrame(const uint8_t* frameBuf, size_t frameSize)
{
    DBG_LINDECODER("\n");
    return false;
}

bool LinuxDecoder::isReady() const
{
    DBG_LINDECODER("\n");
    return mIsReady;
}

bool LinuxDecoder::isStarted() const
{
    DBG_LINDECODER("\n");
    return mIsStarted;
}

void LinuxDecoder::requestSetup()
{
    DBG_LINDECODER("\n");
    // nothing to do
}

bool LinuxDecoder::retrieveFrame()
{
    DBG_LINDECODER("\n");
    return true;
}

void LinuxDecoder::init(void* nativeWindow)
{
    DBG_LINDECODER("\n");

    if (!mDecoderLoop)
    {
        mDecoderLoop.reset(new DecoderLoop(this, mDecMu, mVidFramesToDecode, mDecodedVideoFrames));
    }
}

void LinuxDecoder::diagnostics(void* userData)
{
    DBG_LINDECODER("\n");
    // nothing to do
}

void LinuxDecoder::setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps)
{
    mSps = sps;
    mPps = pps;
}

#endif // LINUX_PLATFORM