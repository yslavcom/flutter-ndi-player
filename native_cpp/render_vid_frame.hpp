#include "interfaces/frame_observer.hpp"

#pragma once

class RenderVidFrame: public RenderVidFrameObserver
{
public:

private:
    virtual void onRender(std::unique_ptr<uint8_t[]>, size_t size) override;
};