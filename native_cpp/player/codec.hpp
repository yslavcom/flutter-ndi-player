
#pragma once

#include <stdint.h>

class Decoder
{
public:
    virtual ~Decoder(){}

    virtual bool create() = 0;
    virtual bool configure() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) = 0;

    virtual bool retrieveFrame() = 0;
};