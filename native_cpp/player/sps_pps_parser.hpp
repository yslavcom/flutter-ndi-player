#pragma once

#include "common/logger.hpp"

#include <stddef.h>
#include <stdint.h>
#include <optional>
#include <array>
#include <vector>

namespace H26x
{

enum class FourCcType
{
    Unknown,
    H264,
    Hevc
};
struct FourCC
{
    FourCC() = delete;

    FourCC(const uint8_t* data)
    {
        word = data[0];
        word = (word << 8) | data[1];
        word = (word << 8) | data[2];
        word = (word << 8) | data[3];
    }

    FourCC(uint32_t rhs)
    {
        word = rhs;
    }

    FourCC(const FourCC& rhs)
    {
        this->word = rhs.word;
    }

    FourCC(FourCC&& rhs)
    {
        this->word = std::move(rhs.word);
        rhs.word = 0;
    }

    FourCC& operator=(const FourCC& rhs)
    {
        this->word = rhs.word;
        return *this;
    }

    FourCC& operator=(const uint32_t& rhs)
    {
        this->word = rhs;
        return *this;
    }

    FourCC& operator=(FourCC&& rhs)
    {
        this->word = std::move(rhs.word);
        rhs.word = 0;
        return *this;
    }

    bool operator==(const char* str)
    {
        uint32_t tmp = (uint32_t)str[0];
        tmp = (tmp << 8) | (uint32_t)str[1];
        tmp = (tmp << 8) | (uint32_t)str[2];
        tmp = (tmp << 8) | (uint32_t)str[3];
        return this->word == tmp;
    }

    bool operator!=(const char* str)
    {
        return !(*this == str);
    }

    operator bool() const
    {
        return word != 0;
    }

    bool isH264() const
    {
        return word == 'H264' || word == 'h264';
    }

    bool isHevc() const
    {
        return word == 'HEVC' || word == 'hevc';
    }

    FourCcType getType() const
    {
        if (isH264()) { return FourCcType::H264; }
        else if (isH264()) { return FourCcType::Hevc; }
        else { return FourCcType::Unknown; }
    }

    static FourCC Undefined()
    {
        return FourCC((uint32_t)0);
    }

    uint32_t word;
};

struct ServiceInfo
{
    ServiceInfo()
        : fourCcType(FourCcType::Unknown)
        , isKeyFrame(false)
    {}

    FourCcType fourCcType;
    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    bool isKeyFrame;
};

std::optional<ServiceInfo> tryParseServiceInfo(FourCcType type, const uint8_t * data, size_t sz, size_t hdrSz);

} // namespace H264