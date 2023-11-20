#ifdef LINUX_PLATFORM

#include "codec.hpp"

#include <vector>

#pragma once

using RequestSetupCb = std::function<void(void*, bool isCompressed)>;

class LinuxDecoder final: public Video::Decoder
{
public:
    LinuxDecoder(RequestSetupCb cb);
    virtual ~LinuxDecoder();

    virtual bool create(uint32_t fourcc) override;
    virtual bool configure() override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual void release() override;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) override;
    virtual bool isReady() const override;
    virtual bool isStarted() const override;
    virtual void requestSetup() override;

    virtual bool retrieveFrame() override;

    virtual void init(void* nativeWindow) override;
    virtual void diagnostics(void* userData) override;

    virtual void setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps) override;
};

#endif // #ifdef LINUX_PLATFORM