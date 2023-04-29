#pragma once

#include "interfaces/frame_observer.hpp"

#include <set>

class RenderVidFrame: public RenderVidFrameObserver
{
public:
    RenderVidFrame()
        : mXres(0)
        , mYres(0)
    {}

    void cleanup(uint8_t* ptr);
    void setOutDim(unsigned xRes, unsigned yRes)
    {
        mXres = xRes;
        mYres = yRes;
    }

private:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) override;
    virtual std::pair<unsigned, unsigned> getOutDim() const override;

    std::set<uint8_t*> mCleanupMemPtr;

    unsigned mXres;
    unsigned mYres;
};

RenderVidFrame* getRenderVidFrame();
