#pragma once

#include "codec.hpp"
#include "decoder-loop.hpp"

#include <vector>
#include <string>
#include <memory>

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

class AndroidDecoder final: public Video::Decoder
{
public:
    AndroidDecoder();
    virtual ~AndroidDecoder();

    const char* getFormatPresentation() const;

    // class Decoder
    virtual bool create() override;
    virtual bool configure() override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) override;
    virtual bool isReady() const override;
    virtual bool retrieveFrame() override;
    virtual void init(unsigned xRes, unsigned yRes, void* nativeWindow) override;

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
    bool mIsValid;
    bool mIsReady;
    bool isValid() const;

    std::unique_ptr<DecoderLoop> mDecoderLoop;
};
