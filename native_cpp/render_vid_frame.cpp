#include "render_vid_frame.hpp"

#include "common/logger.hpp"

#include <jni.h>

void RenderVidFrame::onRender(std::unique_ptr<uint8_t[]> frameBytes, size_t size)
{
    if (!frameBytes || !size)
    {
        //nothing to do
        return;
    }

    LOGW("Render:%d\n", size);
}
