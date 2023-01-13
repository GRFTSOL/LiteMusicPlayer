#ifndef GfxRaw_RawImage_h
#define GfxRaw_RawImage_h

#pragma once

#include "../Window/WindowTypes.h"
#include "../ImageLib/RawImageData.h"


class CRawGraph;
class RawImageData;

bool bltRawImage(RawImageData *pImageDst, const CRect &rcDst, RawImageData *pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc);

void fillRect(RawImageData *pImage, int x, int y, int xMax, int yMax, const CColor &clrFill, BlendPixMode bpm, int nOpacityFill);

/**
 * CRawImage 将图片文件加载到内存中，然后可绘制到 CRawGraph 或者其他的 CRawImage 中.
 *
 * - 绘制的坐标为逻辑坐标
 * - m_x, m_y, m_cx, m_cy 也都为逻辑坐标
 * - 绘制时自动根据 canvas 的 scaleFactor 选择合适的普通/高清图片
 */
class CRawImage {
private:
    CRawImage(const CRawImage &other);
    CRawImage &operator=(const CRawImage &other);

public:
    CRawImage();
    CRawImage(const RawImageDataPtr &image);
    virtual ~CRawImage();

    bool isValid() const { return m_image != nullptr; }

    bool load(cstr_t szFile);

    virtual void attach(const RawImageDataPtr &image);
    virtual void detach();

    // 根据 scaleFactor 获取 RawImageData.
    virtual const RawImageDataPtr &getRawImageData(float scaleFactor) { return m_image; }

    const RawImageDataPtr &getHandle() const { return m_image; }

    void blt(CRawGraph *canvas, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND)
        { blt(canvas, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm); }
    bool blt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND);

    // Use mask to blt image
    void maskBlt(CRawGraph *canvas, int xDest, int yDest, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND)
        { maskBlt(canvas, xDest, yDest, m_cx, m_cy, m_x, m_y, pImgMask, bpm); }
    bool maskBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *pImgMask, BlendPixMode bpm = BPM_BLEND);

    void blt(CRawImage *canvas, int xDest, int yDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255)
        { blt(canvas, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm, nOpacitySrc); }
    bool blt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

    void stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND)
        { stretchBlt(canvas, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm); }
    bool stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND);

    void stretchBlt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255)
        { stretchBlt(canvas, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm, nOpacitySrc); }
    bool stretchBlt(CRawImage *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm = BPM_BLEND, int nOpacitySrc = 255);

    void xScaleBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int xExtendStart, int xExtendEnd, bool bTile, BlendPixMode bpm = BPM_BLEND);

    void xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
    void xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm = BPM_BLEND);
    void yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
    void yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm = BPM_BLEND);
    void tileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);
    void tileMaskBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, CRawImage *imageMask, BlendPixMode bpm = BPM_BLEND);
    void tileBltEx(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm = BPM_BLEND);

    virtual bool isPixelTransparent(CPoint pt) const;

    int x() const { return m_x; }
    int y() const { return m_y; }
    int width() const { return m_cx; }
    int height() const { return m_cy; }

    void setXYWH(int x, int y, int width, int height)
        { m_x = x; m_y = y; m_cx = width; m_cy = height; }
    void setX(int x) { m_x = x; }
    void setY(int y) { m_y = y; }
    void setWidth(int w) { m_cx = w; }
    void setHeight(int h) { m_cy = h; }

protected:
    template<class _DrawImageFun>
    void tileBltExT(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, _DrawImageFun &drawFunc) {
        if (m_cy <= 0 || m_cx <= 0) {
            return;
        }

        int xSrc, ySrc, x, y;
        int nTileX, nTileY;
        int nW;

        //
        // 先将Y方向的绘画了
        // BBB 0000 0000 0000 AAA
        // CCC 1111 1111 1111 222
        // CCC 1111 1111 1111 222
        // DDD 3333 3333 3333 444

        // BBB
        {
            int nLeftCy;
            xSrc = (xDest - xOrg) % m_cx;
            ySrc = (yDest - yOrg) % m_cy;
            nLeftCy = min(m_cy - ySrc, nHeightDest);
            if (xSrc != 0) {
                nW = min(m_cx - xSrc, nWidthDest);
                drawFunc(
                    xDest, yDest,
                    nW, nLeftCy,
                    m_x + xSrc, m_y + ySrc);
                nTileX = xDest + (m_cx - xSrc);
            } else {
                nTileX = xDest;
            }

            // 0000
            if (ySrc != 0) {
                for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx) {
                    drawFunc(
                        x, yDest,
                        m_cx, nLeftCy,
                        m_x, m_y + ySrc);
                }

                // AAA
                if (x < xDest + nWidthDest) {
                    drawFunc(
                        x, yDest,
                        xDest + nWidthDest - x, nLeftCy,
                        m_x, m_y + ySrc);
                }
                nTileY = yDest + (m_cy - ySrc);
            } else {
                nTileY = yDest;
            }
        }

        for (y = nTileY; y + m_cy < yDest + nHeightDest; y += m_cy) {
            // CCCC
            if (xSrc != 0) {
                nW = min(m_cx - xSrc, nWidthDest);
                drawFunc(
                    xDest, y,
                    nW, m_cy,
                    m_x + xSrc, m_y);
            }

            // 1111
            for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx) {
                drawFunc(
                    x, y,
                    m_cx, m_cy,
                    m_x, m_y);
            }

            // 222
            if (x < xDest + nWidthDest) {
                drawFunc(
                    x, y,
                    xDest + nWidthDest - x, m_cy,
                    m_x, m_y);
            }
        }

        if (y < yDest + nHeightDest) {
            int nLeftCy;

            nLeftCy = yDest + nHeightDest - y;

            // DDD
            if (xSrc != 0) {
                nW = min(m_cx - xSrc, nWidthDest);
                drawFunc(
                    xDest, y,
                    nW, nLeftCy,
                    m_x + xSrc, m_y);
            }

            // 3333
            for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx) {
                drawFunc(
                    x, y,
                    m_cx, nLeftCy,
                    m_x, m_y);
            }

            // 444
            if (x < xDest + nWidthDest) {
                drawFunc(
                    x, y,
                    xDest + nWidthDest - x, nLeftCy,
                    m_x, m_y);
            }
        }
    }

public:
    int                         m_x;
    int                         m_y;
    int                         m_cx;
    int                         m_cy;

protected:
    RawImageDataPtr             m_image;

};

class CDrawImageFun {
public:
    CDrawImageFun(CRawGraph *canvas, CRawImage *image, BlendPixMode bpm) : m_canvas(canvas), m_image(image), m_bpm(bpm) { }

    void operator()(int xdest, int ydest, int width, int height, int xsrc, int ysrc) {
        m_image->blt(m_canvas, xdest, ydest, width, height, xsrc, ysrc, m_bpm);
    }

public:
    CRawImage                   *m_image;
    CRawGraph                   *m_canvas;
    BlendPixMode                m_bpm;

};

class CDrawImageFunMask {
public:
    CDrawImageFunMask(CRawGraph *canvas, CRawImage *image, CRawImage *imageMask, BlendPixMode bpm) {
        m_canvas = canvas;
        m_image = image;
        m_imageMask = imageMask;
        m_bpm = bpm;
    }

    void operator()(int xdest, int ydest, int width, int height, int xsrc, int ysrc) {
        if (m_imageMask) {
            m_image->maskBlt(m_canvas, xdest, ydest, width, height, xsrc, ysrc, m_imageMask, m_bpm);
        } else {
            m_image->blt(m_canvas, xdest, ydest, width, height, xsrc, ysrc, m_bpm);
        }
    }

public:
    CRawImage                   *m_image;
    CRawImage                   *m_imageMask;
    CRawGraph                   *m_canvas;
    BlendPixMode                m_bpm;

};

/************************************************************************

The image with --- is expandable

********---****
********---****
********---****
---------------
---------------
********---****

\************************************************************************/
class CScaleImagePainter {
public:
    CScaleImagePainter() {
        bDrawCenterArea = true;
    }

    int                         srcX, srcY, srcWidth, srcHeight;
    int                         srcHorzExtendStart, srcHorzExtendEnd;
    int                         srcVertExtendStart, srcVertExtendEnd;
    bool                        bDrawCenterArea;

public:
    template<class _DrawImageFun>
    void blt(int nDstX, int nDstY, int nDstW, int nDstH, _DrawImageFun &blter) {
        //             ********---****
        //             ***A****-B-**C*
        //             ********---****
        //             ---D-----E---F-
        //             ---------------
        //             ***G****-H-**J*
        int nDstHorzExtendStart, nDstHorzExtendEnd;
        int nDstVertExtendStart, nDstVertExtendEnd;

        nDstHorzExtendStart = nDstX + srcHorzExtendStart;
        nDstHorzExtendEnd = nDstX + nDstW - (srcWidth - (srcHorzExtendEnd - srcX));
        nDstVertExtendStart = nDstY + srcVertExtendStart;
        nDstVertExtendEnd = nDstY + nDstH - (srcHeight - (srcVertExtendEnd - srcY));

        // draw area A
        blter(nDstX, nDstY,
            nDstHorzExtendStart - nDstX, srcVertExtendStart - srcY,
            srcX, srcY);

        // draw area B
        if (nDstHorzExtendStart < nDstHorzExtendEnd) {
            tileBltT(nDstHorzExtendStart, nDstY,
                nDstHorzExtendEnd - nDstHorzExtendStart,
                srcVertExtendStart - srcY,    // Destination cy
                srcHorzExtendStart, srcY,        // Source x, y
                srcHorzExtendEnd - srcHorzExtendStart,
                srcVertExtendStart - srcY, blter);
        }

        // draw area C
        blter(nDstHorzExtendEnd, nDstY,
            nDstX + nDstW - nDstHorzExtendEnd,
            srcVertExtendStart - srcY,
            srcHorzExtendEnd, srcY);

        //
        // Line 2
        //

        // draw area D
        tileBltT(nDstX, nDstVertExtendStart,
            nDstHorzExtendStart - nDstX,
            nDstVertExtendEnd - nDstVertExtendStart,
            srcX, srcVertExtendStart,
            srcHorzExtendStart - srcX,
            srcVertExtendEnd - srcVertExtendStart, blter);

        // draw area E
        if (bDrawCenterArea) {
            tileBltT(nDstHorzExtendStart, nDstVertExtendStart,
                nDstHorzExtendEnd - nDstHorzExtendStart,
                nDstVertExtendEnd - nDstVertExtendStart,
                srcHorzExtendStart, srcVertExtendStart,
                srcHorzExtendEnd - srcHorzExtendStart,
                srcVertExtendEnd - srcVertExtendStart, blter);
        }

        // draw area F
        tileBltT(nDstHorzExtendEnd, nDstVertExtendStart,
            nDstX + nDstW - nDstHorzExtendEnd,
            nDstVertExtendEnd - nDstVertExtendStart,
            srcHorzExtendEnd, srcVertExtendStart,
            srcX + srcWidth - srcHorzExtendEnd,
            srcVertExtendEnd - srcVertExtendStart, blter);

        //
        // Line 3
        //

        // draw area G
        blter(nDstX, nDstVertExtendEnd,
            nDstHorzExtendStart - nDstX, nDstY + nDstH - srcVertExtendEnd,
            srcX, srcVertExtendEnd);

        // draw area H
        if (nDstHorzExtendStart < nDstHorzExtendEnd) {
            tileBltT(nDstHorzExtendStart, nDstVertExtendEnd,
                nDstHorzExtendEnd - nDstHorzExtendStart,
                nDstY + nDstH - srcVertExtendEnd,    // Destination cy
                srcHorzExtendStart, srcVertExtendEnd,        // Source x, y
                srcHorzExtendEnd - srcHorzExtendStart,
                srcVertExtendStart - srcY, blter);
        }

        // draw area J
        blter(nDstHorzExtendEnd, nDstVertExtendEnd,
            nDstX + nDstW - nDstHorzExtendEnd,
            nDstY + nDstH - srcVertExtendEnd,
            srcHorzExtendEnd, srcVertExtendEnd);
    }

};

template<class _DrawImageFun>
void tileBltT(int xDest, int yDest, int nWidthDest, int nHeightDest, int xSrc, int ySrc, int cxSrcUnit, int cySrcUnit, _DrawImageFun &drawFunc) {
    if (cySrcUnit <= 0 || cxSrcUnit <= 0) {
        return;
    }

    int x, y;

    //
    // 先将Y方向的绘画了
    // 1111 1111 1111 222
    // 1111 1111 1111 222
    // 3333 3333 3333 444
    for (y = yDest; y + cySrcUnit < yDest + nHeightDest; y += cySrcUnit) {
        // 1111
        for (x = xDest; x + cxSrcUnit <= xDest + nWidthDest; x += cxSrcUnit) {
            drawFunc(
                x, y,
                cxSrcUnit, cySrcUnit,
                xSrc, ySrc);
        }

        // 2222
        if (x < xDest + nWidthDest) {
            drawFunc(
                x, y,
                xDest + nWidthDest - x, cySrcUnit,
                xSrc, ySrc);
        }
    }

    if (y < yDest + nHeightDest) {
        int nLeftCy;

        nLeftCy = yDest + nHeightDest - y;

        // 3333
        for (x = xDest; x + cxSrcUnit <= xDest + nWidthDest; x += cxSrcUnit) {
            drawFunc(
                x, y,
                cxSrcUnit, nLeftCy,
                xSrc, ySrc);
        }

        // 444
        if (x < xDest + nWidthDest) {
            drawFunc(
                x, y,
                xDest + nWidthDest - x, nLeftCy,
                xSrc, ySrc);
        }
    }
}

#endif // !defined(GfxRaw_RawImage_h)
