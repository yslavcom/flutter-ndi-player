#pragma once

#include <stdarg.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

void LOGW(const char * format, ... );
void LOGE(const char * format, ... );

#ifdef __cplusplus
}
#endif