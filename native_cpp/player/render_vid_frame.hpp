#pragma once

#include "interfaces/frame_observer.hpp"
#include "player.hpp"
#include "codec.hpp"

#include <set>

class RenderVidFrame: public RenderVidFrameObserver
{
public:
    RenderVidFrame()
        : mXres(0)
        , mYres(0)
        , mVideoDecoder(nullptr)
    {}

    void cleanup(uint8_t* ptr);
    void setOutDim(unsigned xRes, unsigned yRes)
    {
        mXres = xRes;
        mYres = yRes;
    }

    virtual std::pair<unsigned, unsigned> getOutDim() const override;

    void setDecoder(Video::Decoder* decoder);

private:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) override;

    std::set<uint8_t*> mCleanupMemPtr;

    unsigned mXres;
    unsigned mYres;

    friend RenderVidFrame* getRenderVidFrame();
    Video::Decoder* mVideoDecoder;
};

RenderVidFrame* getRenderVidFrame();
Video::Decoder* getVideoDecoder();