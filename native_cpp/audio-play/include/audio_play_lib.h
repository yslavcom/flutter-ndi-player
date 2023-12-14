#pragma once

#ifndef TEST_LIB
#define TEST_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct AudFrameStr
{
    uintptr_t opaque;
    uint32_t chan_no;
    uintptr_t samples_opaque;
    uint32_t samples_no;
    uintptr_t stride;
    bool planar;
};

void audio_setup(void);
bool audio_push_aud_frame(uintptr_t opaque, uint32_t chan_no, uintptr_t samples_opaque, uint32_t samples_no, uintptr_t stride, bool planar);


#ifdef __cplusplus
}
#endif

#endif /* TEST_LIB */