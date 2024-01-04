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

void audio_setup(void(*cb)(void*, void*), uintptr_t context);
bool audio_push_aud_frame(uintptr_t opaque, uint32_t chan_no, uintptr_t samples_opaque, uint32_t samples_no, uintptr_t stride, bool planar);
void audio_delete_all_samples();

#ifdef __cplusplus
}
#endif

#endif /* TEST_LIB */