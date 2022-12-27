#include "SkinTypes.h"
#include "Skin.h"
#include "SkinImage.h"


CSFImage::CSFImage(void) {
    m_pSkinFactory = nullptr;
}

CSFImage::CSFImage(const CSFImage &src) {
    m_pSkinFactory = nullptr;
    copyFrom(src);
}

CSFImage::~CSFImage(void) {
    destroy();
}


void CSFImage::destroy() {
    if (m_pSkinFactory) {
        if (m_image) {
            m_pSkinFactory->getResourceMgr()->freeBitmap(m_image);
            m_image = nullptr;
        }
        m_pSkinFactory = nullptr;
    } else {
        CRawImage::destroy();
    }
}


CSFImage &CSFImage::operator=(const CSFImage &src) {
    copyFrom(src);

    return *this;
}

void CSFImage::copyFrom(const CSFImage &src) {
    if (this == &src) {
        return;
    }

    destroy();

    if (src.m_pSkinFactory && src.m_image) {
        src.m_pSkinFactory->getResourceMgr()->incBitmapReference(src.m_image);
        m_image = src.m_image;
    }

    m_x = src.m_x;
    m_y = src.m_y;
    m_cx = src.m_cx;
    m_cy = src.m_cy;
    m_pSkinFactory = src.m_pSkinFactory;
}

bool CSFImage::loadFromSRM(CSkinFactory *pskinFactory, cstr_t szResName) {
    destroy();

    assert(pskinFactory);
    m_pSkinFactory = pskinFactory;

    RawImageData *image = m_pSkinFactory->getResourceMgr()->loadBitmap(szResName);
    if (image) {
        attach(image);
        if (0 == m_cx && 0 == m_cy) {
            getOrginalSize(m_cx, m_cy);
        }
    } else {
        ERR_LOG1("load Bitmap: %s, FAILED.", szResName);
    }

    return image != nullptr;
}

void CSFImage::xScaleBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int xExtendStart, int xExtendEnd, bool bTile, BlendPixMode bpm) {
    assert(m_cx);
    if (m_cx <= 0) {
        return;
    }

    int x = xDest;
    int xSrc = m_x;

    // start ...
    CRawImage::blt(canvas,
        x, yDest,
        xExtendStart - m_x, m_cy,
        xSrc, m_y, bpm);

    x += xExtendStart - xSrc;

    // Center part
    if (bTile) {
        xTileBlt(canvas, x, yDest,
            nWidthDest - (m_cx - (xExtendEnd - xExtendStart)), m_cy,
            xExtendStart, m_y,
            xExtendEnd - xExtendStart, m_cy, bpm);
    } else {
        CRawImage::stretchBlt(canvas, x, yDest,
            nWidthDest - (m_cx - (xExtendEnd - xExtendStart)), m_cy,
            xExtendStart, m_y,
            xExtendEnd - xExtendStart, m_cy, bpm);
    }

    x = xDest + nWidthDest - (m_x + m_cx - xExtendEnd);

    // End ...
    CRawImage::blt(canvas,
        x, yDest,
        m_x + m_cx - xExtendEnd, m_cy,
        xExtendEnd, m_y, bpm);
}

void CSFImage::xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    assert(m_cx);
    if (m_cx <= 0) {
        return;
    }

    int x;

    for (x = xDest; x + m_cx < xDest + nWidthDest; x += m_cx) {
        CRawImage::blt(canvas,
            x, yDest,
            m_cx, m_cy,
            m_x, m_y, bpm);
    }

    if (x < xDest + nWidthDest) {
        CRawImage::blt(canvas,
            x, yDest,
            xDest + nWidthDest - x, m_cy,
            m_x, m_y, bpm);
    }
}

void CSFImage::xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm) {
    assert(nImageCx);
    if (nImageCx <= 0) {
        return;
    }

    int x;

    for (x = xDest; x + nImageCx < xDest + nWidthDest; x += nImageCx) {
        CRawImage::blt(canvas,
            x, yDest,
            nImageCx, nImageCy,
            nImageX, nImageY, bpm);
    }

    if (x < xDest + nWidthDest) {
        CRawImage::blt(canvas,
            x, yDest,
            xDest + nWidthDest - x, nImageCy,
            nImageX, nImageY, bpm);
    }
}

void CSFImage::yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    assert(m_cy);
    if (m_cy <= 0) {
        return;
    }

    int y;

    for (y = yDest; y + m_cy < yDest + nHeightDest; y += m_cy) {
        CRawImage::blt(canvas,
            xDest, y,
            m_cx, m_cy,
            m_x, m_y, bpm);
    }

    if (y < yDest + nHeightDest) {
        CRawImage::blt(canvas,
            xDest, y,
            m_cx, yDest + nHeightDest - y,
            m_x, m_y, bpm);
    }
}

void CSFImage::yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm) {
    assert(nImageCy);
    if (nImageCy <= 0) {
        return;
    }

    int y;

    for (y = yDest; y + nImageCy < yDest + nHeightDest; y += nImageCy) {
        CRawImage::blt(canvas,
            xDest, y,
            nImageCx, nImageCy,
            nImageX, nImageY, bpm);
    }

    if (y < yDest + nHeightDest) {
        CRawImage::blt(canvas,
            xDest, y,
            nImageCx, yDest + nHeightDest - y,
            nImageX, nImageY, bpm);
    }
}

void CSFImage::tileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    CDrawImageFun fun(canvas, this, bpm);

    tileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

void CSFImage::tileMaskBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, CSFImage *pImageMask, BlendPixMode bpm) {
    CDrawImageFunMask fun(canvas, this, pImageMask, bpm);
    tileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

/*
void CSFImage::tileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm)
{
    assert(m_cy);
    assert(m_cx);

    if (m_cy == 0 || m_cx == 0)
        return;

    int            x, y;

    //
    // 先将Y方向的绘画了
    // 1111 1111 1111 222
    // 1111 1111 1111 222
    // 3333 3333 3333 444
    for (y = yDest; y + m_cy < yDest + nHeightDest; y += m_cy)
    {
        // 1111
        for (x = xDest; x + m_cx <= xDest + nWidthDest; x += m_cx)
        {
            blt(canvas,
                x, y,
                m_cx, m_cy,
                m_x, m_y, bpm);
        }

        // 2222
        if (x < xDest + nWidthDest)
        {
            blt(canvas,
                x, y,
                xDest + nWidthDest - x, m_cy,
                m_x, m_y, bpm);
        }
    }

    if (y < yDest + nHeightDest)
    {
        int        nLeftCy;

        nLeftCy = yDest + nHeightDest - y;

        // 3333
        for (x = xDest; x + m_cx <= xDest + nWidthDest; x += m_cx)
        {
            blt(canvas,
                x, y,
                m_cx, nLeftCy,
                m_x, m_y, bpm);
        }

        // 444
        if (x < xDest + nWidthDest)
        {
            blt(canvas,
                x, y,
                xDest + nWidthDest - x, nLeftCy,
                m_x, m_y, bpm);
        }
    }
}
*/

/*
template<class _DrawImageFun>
void CSkinImage::tileBltExT(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, _DrawImageFun &drawFunc)
{
    assert(m_cy);
    assert(m_cx);

    if (m_cy == 0 || m_cx == 0)
        return;

    int            xSrc, ySrc, x, y;
    int            nTileX, nTileY;
    int            nW;

    //
    // 先将Y方向的绘画了
    // BBB 0000 0000 0000 AAA
    // CCC 1111 1111 1111 222
    // CCC 1111 1111 1111 222
    // DDD 3333 3333 3333 444

    // BBB
    {
        int        nLeftCy;
        xSrc = (xDest - xOrg) % m_cx;
        ySrc = (yDest - yOrg) % m_cy;
        nLeftCy = min(m_cy - ySrc, nHeightDest);
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            drawFunc(this,
                xDest, yDest,
                nW, nLeftCy,
                m_x + xSrc, m_y + ySrc);
            nTileX = xDest + (m_cx - xSrc);
        }
        else
            nTileX = xDest;

        // 0000
        if (ySrc != 0)
        {
            for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
            {
                drawFunc(this,
                    x, yDest,
                    m_cx, nLeftCy,
                    m_x, m_y + ySrc);
            }

            // AAA
            if (x < xDest + nWidthDest)
            {
                drawFunc(this,
                    x, yDest,
                    xDest + nWidthDest - x, nLeftCy,
                    m_x, m_y + ySrc);
            }
            nTileY = yDest + (m_cy - ySrc);
        }
        else
            nTileY = yDest;
    }

    for (y = nTileY; y + m_cy < yDest + nHeightDest; y += m_cy)
    {
        // CCCC
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            drawFunc(this,
                xDest, y,
                nW, m_cy,
                m_x + xSrc, m_y);
        }

        // 1111
        for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx)
        {
            drawFunc(this,
                x, y,
                m_cx, m_cy,
                m_x, m_y);
        }

        // 222
        if (x < xDest + nWidthDest)
        {
            drawFunc(this,
                x, y,
                xDest + nWidthDest - x, m_cy,
                m_x, m_y);
        }
    }

    if (y < yDest + nHeightDest)
    {
        int        nLeftCy;

        nLeftCy = yDest + nHeightDest - y;

        // DDD
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            drawFunc(this,
                xDest, y,
                nW, nLeftCy,
                m_x + xSrc, m_y);
        }

        // 3333
        for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
        {
            drawFunc(this,
                x, y,
                m_cx, nLeftCy,
                m_x, m_y);
        }

        // 444
        if (x < xDest + nWidthDest)
        {
            drawFunc(this,
                x, y,
                xDest + nWidthDest - x, nLeftCy,
                m_x, m_y);
        }
    }
}*/

void CSFImage::tileBltEx(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    CDrawImageFun fun(canvas, this, bpm);

    tileBltExT(canvas, xOrg, yOrg, xDest, yDest, nWidthDest, nHeightDest, fun);
}

/*
void CSFImage::tileBltEx(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest)
{
    assert(m_cy);
    assert(m_cx);

    if (m_cy == 0 || m_cx == 0)
        return;

    int            xSrc, ySrc, x, y;
    int            nTileX, nTileY;
    int            nW;

    //
    // 先将Y方向的绘画了
    // BBB 0000 0000 0000 AAA
    // CCC 1111 1111 1111 222
    // CCC 1111 1111 1111 222
    // DDD 3333 3333 3333 444

    // BBB
    {
        int        nLeftCy;
        xSrc = (xDest - xOrg) % m_cx;
        ySrc = (yDest - yOrg) % m_cy;
        nLeftCy = min(m_cy - ySrc, nHeightDest);
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            canvas->DrawImage(this,
                xDest, yDest,
                nW, nLeftCy,
                m_x + xSrc, m_y + ySrc);
            nTileX = xDest + (m_cx - xSrc);
        }
        else
            nTileX = xDest;

        // 0000
        if (ySrc != 0)
        {
            for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
            {
                canvas->DrawImage(this,
                    x, yDest,
                    m_cx, nLeftCy,
                    m_x, m_y + ySrc);
            }

            // AAA
            if (x < xDest + nWidthDest)
            {
                canvas->DrawImage(this,
                    x, yDest,
                    xDest + nWidthDest - x, nLeftCy,
                    m_x, m_y + ySrc);
            }
            nTileY = yDest + (m_cy - ySrc);
        }
        else
            nTileY = yDest;
    }

    for (y = nTileY; y + m_cy < yDest + nHeightDest; y += m_cy)
    {
        // CCCC
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            canvas->DrawImage(this,
                xDest, y,
                nW, m_cy,
                m_x + xSrc, m_y);
        }

        // 1111
        for (x = nTileX; x + m_cx < xDest + nWidthDest; x += m_cx)
        {
            canvas->DrawImage(this,
                x, y,
                m_cx, m_cy,
                m_x, m_y);
        }

        // 222
        if (x < xDest + nWidthDest)
        {
            canvas->DrawImage(this,
                x, y,
                xDest + nWidthDest - x, m_cy,
                m_x, m_y);
        }
    }

    if (y < yDest + nHeightDest)
    {
        int        nLeftCy;

        nLeftCy = yDest + nHeightDest - y;

        // DDD
        if (xSrc != 0)
        {
            nW = min(m_cx - xSrc, nWidthDest);
            canvas->DrawImage(this,
                xDest, y,
                nW, nLeftCy,
                m_x + xSrc, m_y);
        }

        // 3333
        for (x = nTileX; x + m_cx <= xDest + nWidthDest; x += m_cx)
        {
            canvas->DrawImage(this,
                x, y,
                m_cx, nLeftCy,
                m_x, m_y);
        }

        // 444
        if (x < xDest + nWidthDest)
        {
            canvas->DrawImage(this,
                x, y,
                xDest + nWidthDest - x, nLeftCy,
                m_x, m_y);
        }
    }
}*/
