#include "logger.hpp"

/////////////////////////////////////////////
#ifdef ANDROID_PLATFORM

#include <android/log.h>
#include <stdarg.h>

#define LOG_TAG ""

// void LOGV(const char * format, ... ) { __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__); }
// void LOGD(const char * format, ... ) { __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__); }
// void LOGI(const char * format, ... ) { __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__); }
void LOGW(const char * format, ... )
{
    va_list arg;
    va_start(arg, format);
    char str[256];
    vsnprintf(str, 256, format, arg);
    __android_log_print(ANDROID_LOG_WARN, LOG_TAG, "%s", str);
    va_end(arg);
}

void LOGE(const char * format, ... )
{
    va_list arg;
    va_start(arg, format);
    char str[256];
    vsnprintf(str, 256, format, arg);
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", str);
    va_end(arg);
}

// void LOGSIMPLE(const char * format, ... ) {}
#endif

/////////////////////////////////////////////
#ifdef LINUX_PLATFORM

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
