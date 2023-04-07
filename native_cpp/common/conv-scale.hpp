
#pragma once

#include <cstddef>
#include <cstdint>

struct ScaleContext;

class ConvertScale
{
public:
    ConvertScale();
    ConvertScale(unsigned srcW, unsigned srcH, unsigned dstW, unsigned dstH);
    ~ConvertScale();

    int scale(unsigned srcH, const uint8_t *const srcSlice[], const int srcStride[], uint8_t *const dst[], const int dstStride[]);
    int scale(unsigned dstW, unsigned dstH, unsigned srcW, unsigned srcH, const uint8_t *const srcSlice[], const int srcStride[], uint8_t *const dst[], const int dstStride[]);

private:
    ScaleContext* mScaleContext;
};