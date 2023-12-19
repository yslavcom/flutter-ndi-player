#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

void LOGW(const char * format, ... );
void LOGE(const char * format, ... );

#if 0
#define ASSERT(condition) { if(!(condition)){ std::cerr << "ASSERT FAILED: " << #condition << " @ " << __FILE__ << " (" << __LINE__ << ")" << std::endl; } }
#endif

#ifdef __cplusplus
}

std::string logFourcc(uint32_t fourcc);

#endif