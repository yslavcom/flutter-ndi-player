#pragma once

#include "codec.hpp"
#include <vector>
#include <string>

class AMediaCodec;
class AMediaFormat;
class ANativeWindow;

/*
typedef struct AMediaCodecOnAsyncNotifyCallback {
      AMediaCodecOnAsyncInputAvailable  onAsyncInputAvailable;
      AMediaCodecOnAsyncOutputAvailable onAsyncOutputAvailable;
      AMediaCodecOnAsyncFormatChanged   onAsyncFormatChanged;
      AMediaCodecOnAsyncError           onAsyncError;
} AMediaCodecOnAsyncNotifyCallback;
*/

class AndroidDecoder final: public Decoder
{
public:
    AndroidDecoder(unsigned xRes, unsigned yRex, ANativeWindow* nativeWindow = nullptr);
    virtual ~AndroidDecoder();

    const char* getFormatPresentation() const;

    // class Decoder
    virtual bool create() override;
    virtual bool configure() override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) override;
    virtual bool retrieveFrame() override;

private:
    unsigned mXRes;
    unsigned mYRes;

    AMediaCodec* mCodec;
    AMediaFormat* mFormat;

    const char* mH264Type = "video/avc";

    struct CsdData
    {
        CsdData(const char* _name)
            : name(_name)
        {
        }
        std::vector<uint8_t> data;
        std::string name;
    };
    CsdData mCsdDataSps;
    CsdData mCsdDataPps;

    ANativeWindow* mNativeWindow;

    bool mIsStarted;
};