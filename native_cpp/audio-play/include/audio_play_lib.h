#pragma once

#ifndef TEST_LIB
#define TEST_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t audio_play_adder(uint32_t);
void audio_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_LIB */