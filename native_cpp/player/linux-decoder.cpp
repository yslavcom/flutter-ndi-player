#ifdef LINUX_PLATFORM

#include "linux-decoder.hpp"

LinuxDecoder::LinuxDecoder(RequestSetupCb cb)
{}

LinuxDecoder::~LinuxDecoder()
{}

bool LinuxDecoder::create(uint32_t fourcc)
{
    return false;
}

bool LinuxDecoder::configure()
{
    return false;
}

bool LinuxDecoder::start()
{
    return false;
}

bool LinuxDecoder::stop()
{
    return true;
}

void LinuxDecoder::release()
{}

bool LinuxDecoder::enqueueFrame(const uint8_t* frameBuf, size_t frameSize)
{
    return false;
}

bool LinuxDecoder::isReady() const
{
    return false;
}

bool LinuxDecoder::isStarted() const
{
    return false;
}

void LinuxDecoder::requestSetup()
{}

bool LinuxDecoder::retrieveFrame()
{
    return false;
}

void LinuxDecoder::init(void* nativeWindow)
{}

void LinuxDecoder::diagnostics(void* userData)
{}

void LinuxDecoder::setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps)
{}

#endif // LINUX_PLATFORM