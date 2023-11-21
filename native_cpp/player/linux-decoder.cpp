#ifdef LINUX_PLATFORM

#include "linux-decoder.hpp"

#define _DBG_LINDECODER

#ifdef _DBG_LINDECODER
    #define DBG_LINDECODER(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_LINDECODER(format, ...)
#endif

LinuxDecoder::LinuxDecoder(RequestSetupCb cb)
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
    return false;
}

bool LinuxDecoder::start()
{
    DBG_LINDECODER("\n");
    return false;
}

bool LinuxDecoder::stop()
{
    DBG_LINDECODER("\n");
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
    return false;
}

bool LinuxDecoder::isStarted() const
{
    DBG_LINDECODER("\n");
    return false;
}

void LinuxDecoder::requestSetup()
{
    DBG_LINDECODER("\n");
}

bool LinuxDecoder::retrieveFrame()
{
    DBG_LINDECODER("\n");
    return false;
}

void LinuxDecoder::init(void* nativeWindow)
{
    DBG_LINDECODER("\n");
}

void LinuxDecoder::diagnostics(void* userData)
{
    DBG_LINDECODER("\n");
}

void LinuxDecoder::setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps)
{
    DBG_LINDECODER("\n");
}

#endif // LINUX_PLATFORM