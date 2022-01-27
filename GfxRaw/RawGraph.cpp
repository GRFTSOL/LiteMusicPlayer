// RawGraph.cpp: implementation of the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "../third-parties/Agg/include/agg_path_storage.h"
#include "../third-parties/Agg/include/agg_scanline_u.h"
#include "../third-parties/Agg/include/agg_conv_stroke.h"
#include "../third-parties/Agg/include/agg_renderer_mclip.h"

#include "GfxRaw.h"
#include "RawGraph.h"
#include "ImageBuffBlt.h"


//////////////////////////////////////////////////////////////////////////

CRawGraph::CRawGraph()
{
    m_rawBmpFont = nullptr;

    m_clrText.set(RGB(255, 255, 255));
    m_nOpacityPainting = 255;
    m_ptOrigin.x = m_ptOrigin.y = 0;

    memset(&m_rcClip, 0 , sizeof(m_rcClip));
    
    m_bAlphaChannelEnabled = true;
}

CRawGraph::~CRawGraph()
{
}

bool CRawGraph::create(int cx, int cy, WindowHandleHolder *windowHandle, int nBitCount)
{
    if (!CRawGraphData::create(cx, cy, windowHandle, nBitCount))
        return false;

    m_rcClip.left = m_rcClip.top = 0;
    m_rcClip.right = cx;
    m_rcClip.bottom = cy;

    m_ptOrigin.x = m_ptOrigin.y = 0;

    m_bAlphaChannelEnabled = (nBitCount == 32);

    return true;
}

void CRawGraph::line(int x1, int y1, int x2, int y2)
{
    typedef agg::scanline_p8 scanline_type;
    scanline_type sl;
    agg::rasterizer_scanline_aa<> ras;

    x1 = mapX(x1);
    x2 = mapX(x2);
    y1 = mapY(y1);
    y2 = mapY(y2);

    agg::path_storage ps;
    agg::conv_stroke<agg::path_storage> pg(ps);
    agg::rgba    clr(m_pen.m_clrPen.r() / (double)255, 
        m_pen.m_clrPen.g() / (double)255, 
        m_pen.m_clrPen.b() / (double)255,
        m_pen.m_clrPen.a() * m_nOpacityPainting / 255 / (double)255);

    pg.width(m_pen.m_nWidth);

    ps.remove_all();
    ps.move_to(x1, y1);
    ps.line_to(x2, y2);
    ras.add_path(pg);

    if (m_imageData.bitCount == 24)
    {
        typedef agg::pixfmt_rgb24    pixfmt;
        typedef agg::renderer_mclip<pixfmt> renderer_mclip;

        agg::rendering_buffer    buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
        pixfmt    pixf(buf);
        renderer_mclip rb(pixf);
        rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

        agg::render_scanlines_aa_solid(ras, sl, rb, clr);
    }
    else if (m_imageData.bitCount == 32)
    {
        typedef agg::pixfmt_rgba32    pixfmt;
        typedef agg::renderer_mclip<pixfmt> renderer_mclip;

        agg::rendering_buffer    buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
        pixfmt    pixf(buf);
        renderer_mclip rb(pixf);
        rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

        agg::render_scanlines_aa_solid(ras, sl, rb, clr);
    }
}

void CRawGraph::rectangle(int x, int y, int width, int height)
{
    typedef agg::scanline_p8 scanline_type;

    x = mapX(x);
    y = mapY(y);

    scanline_type sl;
    agg::rasterizer_scanline_aa<> ras;

    agg::path_storage ps;
    agg::conv_stroke<agg::path_storage> pg(ps);
    agg::rgba    clr(m_pen.m_clrPen.r() / (double)255, 
        m_pen.m_clrPen.g() / (double)255, 
        m_pen.m_clrPen.b() / (double)255,
        m_pen.m_clrPen.a() * m_nOpacityPainting / 255 / (double)255);

    pg.width(m_pen.m_nWidth);

    ps.remove_all();
    ps.move_to(x, y);
    ps.line_to(x + width, y);
    ps.line_to(x + width, y + height);
    ps.line_to(x, y + height);
    ps.line_to(x, y);
    ras.add_path(pg);

    if (m_imageData.bitCount == 24)
    {
        typedef agg::pixfmt_rgb24    pixfmt;
        typedef agg::renderer_mclip<pixfmt> renderer_mclip;

        agg::rendering_buffer    buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
        pixfmt    pixf(buf);
        renderer_mclip rb(pixf);
        rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

        agg::render_scanlines_aa_solid(ras, sl, rb, clr);
    }
    else if (m_imageData.bitCount == 32)
    {
        typedef agg::pixfmt_rgba32    pixfmt;
        typedef agg::renderer_mclip<pixfmt> renderer_mclip;

        agg::rendering_buffer    buf(m_imageData.buff, m_imageData.width, m_imageData.height, m_imageData.stride);
        pixfmt    pixf(buf);
        renderer_mclip rb(pixf);
        rb.add_clip_box(m_rcClip.left, m_rcClip.top, m_rcClip.right, m_rcClip.bottom);

        agg::render_scanlines_aa_solid(ras, sl, rb, clr);
    }
}

void CRawGraph::setPen(CRawPen &pen)
{
    m_pen = pen;
}

void CRawGraph::getClipBoundBox(CRect &rc)
{
    rc = m_rcClip;
}

void CRawGraph::setClipBoundBox(const CRect &rc)
{
    if (rc.left > m_rcClip.left)
        m_rcClip.left = rc.left;
    if (rc.top > m_rcClip.top)
        m_rcClip.top = rc.top;

    if (rc.right < m_rcClip.right)
        m_rcClip.right = rc.right;
    if (rc.bottom < m_rcClip.bottom)
        m_rcClip.bottom = rc.bottom;


    if (m_rcClip.right < m_rcClip.left)
        m_rcClip.right = m_rcClip.left;
    if (m_rcClip.bottom < m_rcClip.top)
        m_rcClip.bottom = m_rcClip.top;
}

void CRawGraph::resetClipBoundBox(const CRect &rc)
{
    m_rcClip = rc;
    if (mapX(m_rcClip.left) < 0)
        m_rcClip.left = revertMapX(0);
    if (mapY(m_rcClip.top) < 0)
        m_rcClip.top = revertMapY(0);
    if (mapX(m_rcClip.right) > width())
        m_rcClip.right = revertMapX(width());
    if (mapY(m_rcClip.bottom) > height())
        m_rcClip.bottom = revertMapY(height());

    if (m_rcClip.right < m_rcClip.left)
        m_rcClip.right = m_rcClip.left;
    if (m_rcClip.bottom < m_rcClip.top)
        m_rcClip.bottom = m_rcClip.top;
}

void CRawGraph::clearClipBoundBox()
{
    m_rcClip.left = revertMapX(0);
    m_rcClip.top = revertMapY(0);
    m_rcClip.right = revertMapX(width());
    m_rcClip.bottom = revertMapY(height());
}

bool CRawGraph::textOut(int x, int y, cstr_t szText, int nLen)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    if (m_rawBmpFont)
        return m_rawBmpFont->textOut(this, mapX(x), mapY(y), m_clrText, szText, nLen, m_bAlphaChannelEnabled);
    return false;
}

bool CRawGraph::drawTextClip(cstr_t szText, int nLen, const CRect &rcPos, int xLeftClipOffset)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    if (m_rawBmpFont)
    {
        return m_rawBmpFont->drawText(this, mapX(rcPos.left), mapY(rcPos.top), mapX(rcPos.right - rcPos.left), mapX(xLeftClipOffset), m_clrText, szText, nLen, m_bAlphaChannelEnabled);
    }
    return false;
}

bool CRawGraph::textOutOutlined(int x, int y, cstr_t szText, int nLen, const CColor &clrText, const CColor &clrBorder)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    if (m_rawBmpFont)
        return m_rawBmpFont->outlinedTextOut(this, mapX(x), mapY(y), clrText, clrBorder, szText, nLen, m_bAlphaChannelEnabled);
    return false;
}

bool CRawGraph::drawTextClipOutlined(cstr_t szText, int nLen, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, int xLeftClipOffset)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    if (m_rawBmpFont)
        return m_rawBmpFont->outlinedDrawText(this, mapX(rcPos.left), mapY(rcPos.top), mapX(rcPos.right - rcPos.left), mapX(xLeftClipOffset), clrText, clrBorder, szText, nLen, m_bAlphaChannelEnabled);
    return false;
}


bool CRawGraph::drawTextOutlined(cstr_t szText, int nLen, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, uint32_t uFormat)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    CRect    rc = rcPos;
    mapRect(rc);

    if (m_rawBmpFont)
        return m_rawBmpFont->outlinedDrawTextEx(this, rc, clrText, clrBorder, szText, nLen, uFormat, m_bAlphaChannelEnabled);

    return false;
}


bool CRawGraph::drawText(cstr_t szText, int nLen, const CRect &rcPos, uint32_t uFormat)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    CRect    rc = rcPos;
    mapRect(rc);

    if (m_rawBmpFont)
        return m_rawBmpFont->drawTextEx(this, rc, m_clrText, szText, nLen, uFormat, m_bAlphaChannelEnabled);

    return false;
}


bool CRawGraph::getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize)
{
    if (nLen == -1)
        nLen = (int)strlen(szText);

    if (m_rawBmpFont)
        return m_rawBmpFont->getTextExtentPoint32(szText, nLen, pSize);

    return false;
}

void CRawGraph::setFont(CRawBmpFont *font)
{
    assert(font);
    if (!font)
        return;

    m_rawBmpFont = font;
}

void CRawGraph::resetAlphaChannel(uint8_t byAlpha, const CRect *lpRect)
{
    if (!m_bAlphaChannelEnabled)
        return;
    
    assert(m_imageData.bitCount == 32);

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    int            nStrideModify;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    // Modify alpha channel directly
    pRow = m_imageData.pixPtr(x, y) + PIX_A;
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p += PIX_SIZE)
        {
            *p = byAlpha;
        }
        pRow += m_imageData.stride;
    }
}

void CRawGraph::multiplyAlpha(uint8_t byAlpha, const CRect *lpRect)
{
    if (!m_bAlphaChannelEnabled)
        return;

    assert(m_imageData.bitCount == 32);

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    int            nStrideModify;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    // PIX_SIZE is the pixel size(rgba).
    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p ++)
        {
            *p = (uint8_t)((*p * byAlpha) / 256);
        }
        pRow += m_imageData.stride;
    }
}

void CRawGraph::vertAlphaFadeOut(const CRect *lpRect, bool bTop)
{
    if (!m_bAlphaChannelEnabled)
        return;

    assert(m_imageData.bitCount == 32);
    if (lpRect->bottom == lpRect->top)
        return;

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    float        fStep, fStart;
    int            nStrideModify;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    fStep = (float)1 / (lpRect->bottom - lpRect->top);
    if (bTop)
    {
        fStart = 0;
    }
    else
    {
        fStep = -fStep;
        fStart = 1;
    }

    fStart += fStep * (y - mapY(lpRect->top));
    assert(fStart <= 1.0 && fStart >= 0.0);

    // PIX_SIZE is the pixel size(rgba).
    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p ++)
        {
            *p = (uint8_t)(*p * fStart);
        }
        pRow += m_imageData.stride;
        fStart += fStep;
    }
}

void CRawGraph::vertFadeOut(const CRect *lpRect, const CColor &clrBg, bool bTop)
{
    assert(m_imageData.bitCount == 32);
    if (lpRect->bottom == lpRect->top)
        return;

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    // float        fStep, fStart;
    int            r, g, b, a;
    int            nStrideModify;

    a = clrBg.getAlpha() * m_nOpacityPainting / 255;
    r = clrBg.r() * a / 255;
    g = clrBg.g() * a / 255;
    b = clrBg.b() * a / 255;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;

    nStrideModify = (xMax - x) * PIX_SIZE;

    int            nFadeRange = lpRect->bottom - lpRect->top;
    int            nFadeStart = y - mapY(lpRect->top);
    
    if (!bTop)
        nFadeStart = nFadeRange - nFadeStart;

/*    fStep = (float)1 / (lpRect->bottom - lpRect->top);
    if (bTop)
    {
        fStart = 0;
    }
    else
    {
        fStep = -fStep;
        fStart = 1;
    }

    fStart += fStep * (y - mapY(lpRect->top));
    assert(fStart <= 1.0 && fStart >= 0.0);*/

    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p += PIX_SIZE)
        {
            // a * f + b * (1 - f) = (a - b) * f + b
            p[PIX_B] = (uint8_t)((p[PIX_B] - b) * nFadeStart / nFadeRange + b);
            p[PIX_G] = (uint8_t)((p[PIX_G] - g) * nFadeStart / nFadeRange + g);
            p[PIX_R] = (uint8_t)((p[PIX_R] - r) * nFadeStart / nFadeRange + r);
            p[PIX_A] = (uint8_t)((p[PIX_A] - a) * nFadeStart / nFadeRange + a);
/*            p[PIX_B] = (uint8_t)((p[PIX_B] - b) * fStart + b);
            p[PIX_G] = (uint8_t)((p[PIX_G] - g) * fStart + g);
            p[PIX_R] = (uint8_t)((p[PIX_R] - r) * fStart + r);
            p[PIX_A] = (uint8_t)((p[PIX_A] - a) * fStart + a);*/
        }
        pRow += m_imageData.stride;
        // fStart += fStep;
        if (bTop)
            nFadeStart++;
        else
            nFadeStart--;
    }
}


void CRawGraph::horzAlphaFadeOut(const CRect *lpRect, bool bLeft)
{
    if (!m_bAlphaChannelEnabled)
        return;

    assert(m_imageData.bitCount == 32);
    if (lpRect->left == lpRect->right)
        return;

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    float        fStep, fStart, f;
    int            nStrideModify;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    fStep = (float)1 / (lpRect->right - lpRect->left);
    if (bLeft)
    {
        fStart = 0;
    }
    else
    {
        fStep = -fStep;
        fStart = 1;
    }

    fStart += fStep * (x - mapX(lpRect->left));
    assert(fStart <= 1.0 && fStart >= 0.0);

    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        f = fStart;

        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p += PIX_SIZE)
        {
            p[PIX_B] = (uint8_t)(p[PIX_B] * f);
            p[PIX_G] = (uint8_t)(p[PIX_G] * f);
            p[PIX_R] = (uint8_t)(p[PIX_R] * f);
            p[PIX_A] = (uint8_t)(p[PIX_A] * f);
            f += fStep;
        }
        pRow += m_imageData.stride;
    }
}


void CRawGraph::horzFadeOut(const CRect *lpRect, const CColor &clrBg, bool bLeft)
{
    assert(m_imageData.bitCount == 32);
    if (lpRect->left == lpRect->right)
        return;

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    float        fStep, fStart, f;
    uint8_t        r, g, b, a;
    int            nStrideModify;

    a = clrBg.getAlpha();
    r = clrBg.r() * a / 255;
    g = clrBg.g() * a / 255;
    b = clrBg.b() * a / 255;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    fStep = (float)1 / (lpRect->right - lpRect->left);
    if (bLeft)
    {
        fStart = 0;
    }
    else
    {
        fStep = -fStep;
        fStart = 1;
    }

    fStart += fStep * (x - mapX(lpRect->left));
    assert(fStart <= 1.0 && fStart >= 0.0);

    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        f = fStart;

        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p += PIX_SIZE)
        {
            p[PIX_B] = (uint8_t)((p[PIX_B] - b) * f + b);
            p[PIX_G] = (uint8_t)((p[PIX_G] - g) * f + g);
            p[PIX_R] = (uint8_t)((p[PIX_R] - r) * f + r);
            p[PIX_A] = (uint8_t)((p[PIX_A] - a) * f + a);
            f += fStep;
        }
        pRow += m_imageData.stride;
    }
}

void CRawGraph::fillRectXOR(const CRect *lpRect, const CColor &clrFill)
{
    assert(m_imageData.bitCount == 32);

    int            x, y, xMax, yMax;
    uint8_t        *p, *pEnd, *pRow;
    int            nStrideModify;
    uint8_t        r = clrFill.r(), g = clrFill.g(), b = clrFill.b();

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;
    nStrideModify = (xMax - x) * PIX_SIZE;

    pRow = m_imageData.pixPtr(x, y);
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        for (p = pRow; p < pEnd; p += PIX_SIZE)
        {
            p[PIX_B] = p[PIX_B] ^ b;
            p[PIX_G] = p[PIX_G] ^ g;
            p[PIX_R] = p[PIX_R] ^ r;
        }
        pRow += m_imageData.stride;
    }
}


template<class _PixRGBBlender, class _PixAlphaBlender>
void fillRectMask(RawImageData *pImage, int x, int y, int xMax, int yMax, RawImageData *pImageMask, int xMask, int yMask, uint8_t *srcPixel)
{
    uint8_t        *p, *pEnd, *pRow;
    uint8_t        *pMask, *pRowMask;
    int            nStrideModify;
    int            nPixSize, nPixSizeMask;

    if (pImage->bitCount == 32)
        nPixSize = 4;
    else if (pImage->bitCount == 24)
        nPixSize = 3;
    else
    {
        assert(0);
        return;
    }

    if (pImageMask->bitCount == 32)
        nPixSizeMask = 4;
    else if (pImageMask->bitCount == 24)
        nPixSizeMask = 3;
    else
    {
        assert(0);
        return;
    }

    nStrideModify = (xMax - x) * nPixSize;

    pRow = pImage->pixPtr(x, y);
    pRowMask = pImageMask->pixPtr(xMask, yMask);
    for (; y < yMax; y++)
    {
        pEnd = pRow + nStrideModify;
        pMask = pRowMask;
        for (p = pRow; p < pEnd; p += nPixSize)
        {
            if (pMask[3] != 0)
            {
                _PixRGBBlender::blend(p, srcPixel, pMask);
                _PixAlphaBlender::blend(p, srcPixel, pMask);
            }
            pMask += nPixSizeMask;
        }
        pRow += pImage->stride;
        pRowMask += pImageMask->stride;
    }
}


void CRawGraph::fillRect(const CRect *lpRect, RawImageData *pImgMask, const CRect *rcMask, const CColor &clrFill, BlendPixMode bpm)
{
    assert(m_imageData.bitCount == 32 || m_imageData.bitCount == 24);

    int            x, y, xMax, yMax;
    uint8_t        srcPixel[4];

    srcPixel[PIX_R] = clrFill.r() * clrFill.getAlpha() / 255;
    srcPixel[PIX_G] = clrFill.g() * clrFill.getAlpha() / 255;
    srcPixel[PIX_B] = clrFill.b() * clrFill.getAlpha() / 255;
    srcPixel[PIX_A] = clrFill.getAlpha();

    assert(pImgMask);
    if (!pImgMask)
        return;

    if (pImgMask->pixFormat != PF_RGBA32)
        return;

    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;

    if (!m_bAlphaChannelEnabled)
        bpm &= ~BPM_CHANNEL_ALPHA;

    // Fill region must be within mask image
    if (xMax - x > rcMask->right - rcMask->left)
        xMax = rcMask->right - rcMask->left + x;
    if (yMax - y > rcMask->bottom - rcMask->top)
        yMax = rcMask->bottom - rcMask->top + y;

    if (isFlagSet(bpm, BPM_OP_BLEND))
    {
        // blend
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_blend_rgb, mask_blend_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
            else
                ::fillRectMask<mask_blend_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_blend_copy_none, mask_blend_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
    }
    else if (isFlagSet(bpm, BPM_OP_MULTIPLY))
    {
        // Multiply
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_multiply_rgb, mask_multiply_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
            else
                ::fillRectMask<mask_multiply_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_blend_copy_none, mask_multiply_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
    }
    else
    {
        // copy
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_copy_rgb, mask_copy_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
            else
                ::fillRectMask<mask_copy_rgb, mask_blend_copy_none>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRectMask<mask_blend_copy_none, mask_copy_alpha>(&m_imageData, x, y, xMax, yMax, pImgMask, rcMask->left, rcMask->top, srcPixel);
        }
    }
}


template<class _PixRGBBlender, class _PixAlphaBlender, class _PixRGBBlender_Opacity, class _PixAlphaBlender_Opacity>
void fillRect(RawImageData *pImage, int x, int y, int xMax, int yMax, uint8_t *srcPixel, uint8_t alpha)
{
    uint8_t        *p, *pEnd, *pRow;
    int            nStrideModify;
    int            nPixSize;

    if (pImage->bitCount == 32)
        nPixSize = 4;
    else if (pImage->bitCount == 24)
        nPixSize = 3;
    else
    {
        assert(0);
        return;
    }

    nStrideModify = (xMax - x) * nPixSize;
    pRow = pImage->pixPtr(x, y);

    if (alpha == 255)
    {
        for (; y < yMax; y++)
        {
            pEnd = pRow + nStrideModify;
            for (p = pRow; p < pEnd; p += nPixSize)
            {
                _PixRGBBlender::blend(p, srcPixel);
                _PixAlphaBlender::blend(p, srcPixel);
            }
            pRow += pImage->stride;
        }
    }
    else
    {
        for (; y < yMax; y++)
        {
            pEnd = pRow + nStrideModify;
            for (p = pRow; p < pEnd; p += nPixSize)
            {
                _PixRGBBlender_Opacity::blend(p, srcPixel, alpha);
                _PixAlphaBlender_Opacity::blend(p, srcPixel, alpha);
            }
            pRow += pImage->stride;
        }
    }
}

void fillRect(RawImageData *pImage, int x, int y, int xMax, int yMax, const CColor &clrFill, BlendPixMode bpm, int nOpacityFill)
{
    assert(pImage);
    assert(pImage->width >= xMax && pImage->height >= yMax);

    uint8_t        srcPixel[4];
    srcPixel[PIX_R] = clrFill.r() * clrFill.getAlpha() / 255;
    srcPixel[PIX_G] = clrFill.g() * clrFill.getAlpha() / 255;
    srcPixel[PIX_B] = clrFill.b() * clrFill.getAlpha() / 255;
    srcPixel[PIX_A] = clrFill.getAlpha();

    if (isFlagSet(bpm, BPM_OP_BLEND))
    {
        // blend
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<blend_rgb, blend_alpha, blend_rgb_opacity, blend_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
            else
                ::fillRect<blend_rgb, blend_copy_none, blend_rgb_opacity, blend_copy_none_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<blend_copy_none, blend_alpha, blend_copy_none_opacity, blend_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
        }
    }
    else if (isFlagSet(bpm, BPM_OP_MULTIPLY))
    {
        // Multiply
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<multiply_rgb, multiply_alpha, multiply_rgb_opacity, multiply_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
            else
                ::fillRect<multiply_rgb, blend_copy_none, multiply_rgb_opacity, blend_copy_none_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<blend_copy_none, multiply_alpha, blend_copy_none_opacity, multiply_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
        }
    }
    else
    {
        // copy
        if (isFlagSet(bpm, BPM_CHANNEL_RGB))
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<copy_rgb, copy_alpha, copy_rgb_opacity, copy_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
            else
            {
                ::fillRect<copy_rgb, blend_copy_none, copy_rgb_opacity, blend_copy_none_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
                // FillRectCopyRGB(pImage, x, y, xMax, yMax, srcPixel);
            }
        }
        else
        {
            if (isFlagSet(bpm, BPM_CHANNEL_ALPHA))
                ::fillRect<blend_copy_none, copy_alpha, blend_copy_none_opacity, copy_alpha_opacity>(pImage, x, y, xMax, yMax, srcPixel, nOpacityFill);
        }
    }
}

void CRawGraph::fillRect(const CRect *lpRect, const CColor &clrFill, BlendPixMode bpm)
{
    assert(m_imageData.bitCount == 32 || m_imageData.bitCount == 24);

    int            x, y, xMax, yMax;
    clipAndMapRect(lpRect, x, y, xMax, yMax);
    if (y >= yMax || x >= xMax)
        return;

    if (!m_bAlphaChannelEnabled)
        bpm &= ~BPM_CHANNEL_ALPHA;

    ::fillRect(&m_imageData, x, y, xMax, yMax, clrFill, bpm, m_nOpacityPainting);
}

CPoint CRawGraph::setOrigin(const CPoint &ptOrg)
{
    CPoint    ptOrgOld = m_ptOrigin;
    m_ptOrigin = ptOrg;

    m_rcClip.left += ptOrgOld.x - ptOrg.x;
    m_rcClip.right += ptOrgOld.x - ptOrg.x;

    m_rcClip.top += ptOrgOld.y - ptOrg.y;
    m_rcClip.bottom += ptOrgOld.y - ptOrg.y;

    return ptOrgOld;
}


void CRawGraph::clipAndMapRect(const CRect *lpRect, int &x, int &y, int &xMax, int &yMax)
{
    if (!lpRect)
    {
        x = m_rcClip.left;
        y = m_rcClip.top;
        xMax = m_rcClip.right;
        yMax = m_rcClip.bottom;
        mapPoint(x, y);
        mapPoint(xMax, yMax);
        return;
    }

    if (lpRect->left < m_rcClip.left)
        x = m_rcClip.left;
    else
        x = lpRect->left;

    if (lpRect->top < m_rcClip.top)
        y = m_rcClip.top;
    else
        y = lpRect->top;

    if (lpRect->right > m_rcClip.right)
        xMax = m_rcClip.right;
    else
        xMax = lpRect->right;
    if (xMax < x)
        xMax = x;

    if (lpRect->bottom > m_rcClip.bottom)
        yMax = m_rcClip.bottom;
    else
        yMax = lpRect->bottom;
    if (yMax < y)
        yMax = y;

    mapPoint(x, y);
    mapPoint(xMax, yMax);
}
