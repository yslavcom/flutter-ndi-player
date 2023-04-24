#pragma once

#include "interfaces/frame_observer.hpp"

#include <set>

class RenderVidFrame: public RenderVidFrameObserver
{
public:

private:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) override;

    void cleanup(uint8_t* ptr);

    std::set<uint8_t*> mCleanupMemPtr;
};

RenderVidFrame* getRenderVidFrame();
