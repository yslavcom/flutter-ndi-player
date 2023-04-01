/////////////////////////////////////////////
#if ANDROID_OUT

#include <android/log.h>

    // LOGS ANDROID
#include <android/log.h>
#define LOG_TAG ""
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)
#define LOGSIMPLE(...)

#endif
/////////////////////////////////////////////
#if LINUX_OUT

#include <stdio.h>
#define LOGV(...) printf("  ");printf(__VA_ARGS__);
#define LOGD(...) printf("  ");printf(__VA_ARGS__);
#define LOGI(...) printf("  ");printf(__VA_ARGS__);
#define LOGW(...) printf("  * Warning: "); printf(__VA_ARGS__);
#define LOGE(...) printf("  *** Error:  ");printf(__VA_ARGS__);
#define LOGSIMPLE(...) printf(" ");

#endif
