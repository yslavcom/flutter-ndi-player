#include "sps_pps_parser.hpp"
#include "common/logger.hpp"

#include <ctype.h>
#include <cstring>

#define _DBG_SPS
#ifdef _DBG_SPS
    #define DBG_SPS(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_SPS(format, ...)
#endif


namespace H26x{

std::optional<ServiceInfo> tryParseServiceInfo(const uint8_t * data, size_t sz, size_t hdrSz)
{
    FourCC fourcc(data);

    if (fourcc.word == 'H264')
    {
        std::vector<uint16_t> NALStrtIdx;
        std::vector<uint8_t> NALType;

        ServiceInfo si{};
        si.fourcc = fourcc;

#if 0
        printBuf(data, hdrSz);
        printBuf(data+hdrSz, 128);
        printBuf(data+hdrSz+128, 128);
#endif

        // save each NAL in vector
        auto buffer = data + hdrSz;
        auto bufferSize = sz - hdrSz;
        for(auto i = 0 ; i < sz; i++)
        {
            if(i + 3 < bufferSize - 1)
            {
                if(buffer[i] == 0x00 && buffer[i + 1] == 0x00 && buffer[i + 2] == 0x00)  //
                {
                    if(buffer[i + 3] == 0x01 || buffer[i + 3] == 0x02 || buffer[i + 3] == 0x03)
                    {
                        NALStrtIdx.push_back(i);
                        NALType.push_back(buffer[i + 4] & 0x1f);

                        i += 4;
                    }
                }
            }
        }

        unsigned spsSize = 0;
        unsigned spsStartIdx = 0;
        unsigned ppsSize = 0;
        unsigned ppsStartIdx = 0;

        for(auto i = 0; i < NALStrtIdx.size(); i++)
        {
            auto idxEnd = 0;
            if( i == NALStrtIdx.size() - 1)
            {
                idxEnd = bufferSize;
            }
            else
            {
                idxEnd = NALStrtIdx[i + 1];
            }

            if(NALType[i] == 0x07) // SPS
            {
                spsStartIdx = NALStrtIdx[i];
                spsSize = idxEnd - NALStrtIdx[i];
            }
            else if(NALType[i] == 0x08) //PPS
            {
                ppsStartIdx = NALStrtIdx[i];
                ppsSize = idxEnd - NALStrtIdx[i];
            }
        }

//        DBG_SPS("spsSize:%d, ppsSize:%d\n", spsSize, spsSize);

        si.sps.resize(spsSize);
        std::memcpy(si.sps.data(), (uint8_t*)buffer + spsStartIdx, spsSize);

        si.pps.resize(ppsSize);
        std::memcpy(si.pps.data(), (uint8_t*)buffer + ppsStartIdx, ppsSize);

        return std::move(si);
    }

    return {};
}

}