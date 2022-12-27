#ifndef GfxRaw_RawImage_h
#define GfxRaw_RawImage_h

#pragma once

#include "../Window/WindowTypes.h"
#include "../ImageLib/RawImageData.h"


class CRawGraph;
class RawImageData;

bool bltRawImage(RawImageData *pImageDst, CRect &rcDst, RawImageData *pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc);

void fillRect(RawImageData *pImage, int x, int y, int xMax, int yMax, const CColor &clrFill, BlendPixMode bpm, int nOpacityFill);

//
//
//
class CRawImage {
public:
    CRawImage();
    virtual ~CRawImage();

    bool isValid() const { return m_image != nullptr; }

    virtual void destroy();

    bool load(cstr_t szFile);

    void blt(CRawGraph *canvas, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND);
    bool blt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND);

    // Use mask to blt image
    void maskBlt(CRawGraph *canvas, int xDest, int yDest, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND);
    bool maskBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND);

    void blt(CRawImage *canvas, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);
    bool blt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

    void stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND);
    bool stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND);

    void stretchBlt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);
    bool stretchBlt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

    const RawImageData *image() const { return m_image; }

    int x() const { return m_x; }
    int y() const { return m_y; }
    int width() const { return m_cx; }
    int height() const { return m_cy; }

    bool getOrginalSize(int &nWidth, int &nHeight);

    void attach(RawImageData *hImage);
    void detach();

    RawImageData *getHandle() const { return m_image; }

protected:
    bool maskBlt(RawImageData *pImageDst, CRect &rcDst, int xSrc, int ySrc, RawImageData *pImageMask, int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc);

public:
    int                         m_x;
    int                         m_y;
    int                         m_cx;
    int                         m_cy;

protected:
    RawImageData                *m_image;

};

#endif // !defined(GfxRaw_RawImage_h)
