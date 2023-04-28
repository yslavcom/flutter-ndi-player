#pragma once

#include "interfaces/frame_observer.hpp"

#include <set>

class RenderVidFrame: public RenderVidFrameObserver
{
public:
    void cleanup(uint8_t* ptr);

private:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) override;

    std::set<uint8_t*> mCleanupMemPtr;
};

RenderVidFrame* getRenderVidFrame();
