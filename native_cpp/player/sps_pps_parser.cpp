#include "sps_pps_parser.hpp"
#include "common/logger.hpp"

#include <ctype.h>
#include <cstring>
#include <cassert>

// #define _DBG_SPS

#ifdef _DBG_SPS
    #define DBG_SPS(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_SPS(format, ...)
#endif


namespace H26x{

std::optional<ServiceInfo> tryParseServiceInfo(FourCcType type, const uint8_t * data, size_t sz, size_t hdrSz)
{
    switch (type)
    {
    case FourCcType::H264:
    {
        std::vector<uint16_t> NALStrtIdx;
        std::vector<uint8_t> NALType;

        ServiceInfo si{};
        si.fourCcType = type;

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
        bool isKeyFrame = false;

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
            else if(NALType[i] == 0x05)
            {
                isKeyFrame = true;
            }

            DBG_SPS("NALType[%d]:%d\n", i, NALType[i]);
        }

        si.sps.resize(spsSize);
        std::memcpy(si.sps.data(), (uint8_t*)buffer + spsStartIdx, spsSize);

        si.pps.resize(ppsSize);
        std::memcpy(si.pps.data(), (uint8_t*)buffer + ppsStartIdx, ppsSize);

        si.isKeyFrame = isKeyFrame;

        return std::move(si);
    }

    case FourCcType::Hevc:
    {
        assert(0 && "Not yet implemented");
    }

    case FourCcType::Unknown:
    {
        assert(0 && "Shouldn't be here");
    }
    }

    return {};
}

}