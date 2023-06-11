#pragma once

#include "codec.hpp"
#include "decoder-loop.hpp"

#include <media/NdkMediaCodec.h>

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <queue>

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

using RequestSetupCb = std::function<void(void*)>;

class AndroidDecoder final: public Video::Decoder
{
public:
    AndroidDecoder(RequestSetupCb cb);
    virtual ~AndroidDecoder();

    const char* getFormatPresentation() const;

    // class Decoder
    virtual bool create(uint32_t fourcc) override;
    virtual bool configure() override;
    virtual bool start() override;
    virtual bool stop() override;
    virtual bool enqueueFrame(const uint8_t* frameBuf, size_t frameSize) override;
    virtual bool isReady() const override;
    virtual bool retrieveFrame() override;
    virtual void init(void* nativeWindow) override;
    virtual void requestSetup() override;
    virtual void setSpsPps(std::vector<uint8_t> sps, std::vector<uint8_t> pps) override;

private:

    static void onAsyncInputAvailable(AMediaCodec *codec, void *userdata, int32_t index);
    void onAsyncInputAvailable(AMediaCodec *codec, int32_t index);

    static void onAsyncOutputAvailable(AMediaCodec *codec, void *userdata, int32_t index, AMediaCodecBufferInfo *bufferInfo);
    void onAsyncOutputAvailable(AMediaCodec *codec, int32_t index, AMediaCodecBufferInfo *bufferInfo);

    static void onAsyncFormatChanged(AMediaCodec *codec, void *userdata, AMediaFormat *format);
    void onAsyncFormatChanged(AMediaCodec *codec, AMediaFormat *format);

    static void onAsyncError(AMediaCodec *codec, void *userdata, media_status_t error, int32_t actionCode, const char *detail);
    void onAsyncError(AMediaCodec *codec, media_status_t error, int32_t actionCode, const char *detail);

    AMediaCodec* mCodec;
    AMediaFormat* mFormat;

    const char* mH264Type = "video/avc";

    ANativeWindow* mNativeWindow;

    bool mIsStarted;
    bool mIsDecoderLoop;
    bool mIsValid;
    bool mIsReady;
    bool isValid() const;

    std::unique_ptr<DecoderLoop> mDecoderLoop;

    RequestSetupCb mRequestSetupCb;
    std::queue<int32_t> mInputAvailableBufferIdx;
};
