#pragma once

#include <ctype.h>
#include <cstddef>

#define _DBG_PRNT_BUF

#ifdef _DBG_PRNT_BUF
    #define DBG_PRNT_BUF(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_PRNT_BUF(format, ...)
#endif


static void printBuf(const uint8_t * buf, size_t len)
{
#ifdef _DBG_PRNT_BUF
    static char scratch[5*1024];
    static char scratchStr[5*1024];
    int strLen = 0;
    int strStrLen = 0;
    for (size_t i=0; i < len; i++ )
    {
        strLen += snprintf(&scratch[strLen], sizeof(scratch)-strLen, "%02x ", buf[i]);

        if (isalpha(buf[i]))
        {
            strStrLen += snprintf(&scratchStr[strStrLen], sizeof(scratchStr)-strStrLen, "%c", buf[i]);
        }
    }
    DBG_PRNT_BUF("scratch:%s\n", scratch);
    DBG_PRNT_BUF("scratch str:%s\n", scratchStr);
#endif
}