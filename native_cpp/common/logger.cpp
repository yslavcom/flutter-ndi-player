#include "logger.hpp"

/////////////////////////////////////////////
#if ANDROID_OUT

#include <android/log.h>

// void LOGV(const char * format, ... ) { __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__); }
// void LOGD(const char * format, ... ) { __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__); }
// void LOGI(const char * format, ... ) { __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__); }
void LOGW(const char * format, ... ) { __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__); }
void LOGE(const char * format, ... ) { __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__); }
// void LOGSIMPLE(const char * format, ... ) {}
#endif

/////////////////////////////////////////////
#ifdef LINUX_OUT

void LOGW(const char * format, ... )
{
    printf("  * Warning: ");

    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stderr, format, arg_list);
    va_end(arg_list);
}


void LOGE(const char * format, ... )
{
    printf("  *** Error:  ");
    va_list arg_list;
    va_start(arg_list, format);
    vfprintf(stderr, format, arg_list);
    va_end(arg_list);
}

#endif