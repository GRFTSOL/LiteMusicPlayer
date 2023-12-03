#ifndef ImageLib_RawImageData_h
#define ImageLib_RawImageData_h

#pragma once

#include "../Utils/Utils.h"


#ifndef _WIN32
struct RGBQUAD {
    uint8_t                     rgbBlue;
    uint8_t                     rgbGreen;
    uint8_t                     rgbRed;
    uint8_t                     rgbReserved;
};
#endif

enum PixFormat {
    PF_UNKNOWN                  = -1,
    PF_RGBA32,                       // = agg::pix_format_rgba32,
    PF_RGB24,                        // = agg::pix_format_rgb24,
    PF_PALLETE256,
    PF_PALLETE16,
    PF_PALLETE2,
};

typedef uint32_t BlendPixMode;

enum _BlendPixMode {
    // Target Channel
    BPM_CHANNEL_RGB             = 1,
    BPM_CHANNEL_ALPHA           = 1 << 1,

    // Operation
    BPM_OP_COPY                 = 1 << 8,
    BPM_OP_BLEND                = 1 << 9,
    BPM_OP_XOR                  = 1 << 10,
    BPM_OP_MULTIPLY             = 1 << 11,


    // Combined Channel and operaton:


    BPM_COPY                    = BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_COPY,
    BPM_BLEND                   = BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_BLEND,
    BPM_MULTIPLY                = BPM_CHANNEL_RGB | BPM_CHANNEL_ALPHA | BPM_OP_MULTIPLY,

    BPM_BLEND_RGB               = BPM_CHANNEL_RGB | BPM_OP_BLEND,
    BPM_BLEND_ALPHA             = BPM_CHANNEL_ALPHA | BPM_OP_BLEND,

    BPM_COPY_ALPHA              = BPM_CHANNEL_ALPHA | BPM_OP_COPY,
    BPM_COPY_RGB                = BPM_CHANNEL_RGB | BPM_OP_COPY,

    BPM_XOR_RGB                 = BPM_CHANNEL_RGB | BPM_OP_XOR,

    BPM_BILINEAR                = 1 << 17, // available option for stretch blt
};

class RawImageData {
private:
    RawImageData(const RawImageData &other);
    RawImageData &operator=(const RawImageData &other);

public:
    RawImageData();
    virtual ~RawImageData();

    void attach(uint8_t *buf, int nWidth, int nHeight, int nBitCount);
    void exchange(RawImageData *pSrc);
    void detach();

    bool create(int nWidth, int nHeight, int nBitCount);
    bool createReverse(int nWidth, int nHeight, int nBitCount);

    void reverseUpDown() { stride = -stride; }

    void free();

    void setClrUsed(int _nClrUsed);

    uint8_t *rowPtr(int y) const {
        if (stride > 0) {
            return buff + stride * y;
        } else {
            return buff - stride * (height - 1 - y);
        }
    }

    uint8_t *pixPtr(int x, int y) const {
        uint8_t *p;

        p = rowPtr(y);
        switch (pixFormat) {
        case PF_RGBA32:
            p += x * 4;
            break;
        case PF_RGB24:
            p += x * 3;
            break;
        case PF_PALLETE256:
            p += x;
            break;
        case PF_PALLETE16:
        case PF_PALLETE2:
            p += x * bitCount / 8;
            break;
        default:
            break;
        }

        return p;
    }

    RGBQUAD getPixel(int x, int y) const;

    uint32_t getBuffSize() { return (stride > 0 ? stride : -stride) * (uint32_t)height; }

    int absStride() { return stride > 0 ? stride : -stride; }

public:
    uint8_t                     *buff;
    uint16_t                    width, height;
    int16_t                     stride;
    int16_t                     bitCount;
    PixFormat                   pixFormat;

    uint8_t                     nClrUsed;
    RGBQUAD                     *pallete;

};

using RawImageDataPtr = std::shared_ptr<RawImageData>;

RawImageDataPtr createRawImageData(int width, int height, int bitCount);

RawImageDataPtr loadRawImageDataFromFile(cstr_t szFile);
RawImageDataPtr loadRawImageDataFromMem(const void *buf, int nSize);

cstr_t guessPictureDataExt(const StringView &imageData);

RawImageDataPtr convertTo24BppRawImage(const RawImageDataPtr &src);

RawImageDataPtr duplicateRawImage(RawImageData *src);

void rawImageBGR24Set(RawImageData *image, uint8_t r, uint8_t g, uint8_t b);
void rawImageBGRA32Set(RawImageData *image, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void rawImageSet(RawImageData *image, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void adjustImageHue(RawImageData *pImage, float hueOffset);
void adjustImageHue(RawImageData *pImage, float hueOffset, float saturationRatio, float luminanceRatio);

void adjustColorHue(COLORREF &clr, float hueOffset);
void adjustColorHue(COLORREF &clr, float hueOffset, float saturationRatio, float luminanceRatio);

COLORREF HLSToRGB(float hue, float saturation, float luminance);
void RGBToHLS(COLORREF clr, float &hue, float &saturation, float &luminance);

#endif // !defined(ImageLib_RawImageData_h)
