#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"

#include "GfxRaw.h"
#include "RawImage.h"

#include "ImageBuffBlt.h"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void stretch_buff_bilinear_bgra32(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, agg::rect_f &rcSrc) {
    typedef agg::pixfmt_bgra32 pixfmt;
    typedef pixfmt::color_type color_type;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgra32_pre pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    agg::scanline_u8 g_scanline;
    agg::rasterizer_scanline_aa<> g_rasterizer;
    pixfmt pixf(bufDest);
    pixfmt_pre pixf_pre(bufDest);
    renderer_base rb(pixf);
    renderer_base_pre rb_pre(pixf_pre);

    renderer_solid r(rb);


    rb.clear(agg::rgba(1, 1, 1));

    g_rasterizer.clip_box(0, 0, bufDest.width(), bufDest.height());
    g_rasterizer.reset();
    g_rasterizer.move_to_d(rcDest.left, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.bottom);
    g_rasterizer.line_to_d(rcDest.left, rcDest.bottom);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    pixfmt pixf_img(bufSrc);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_img);

    double ploygon[8];
    int g_x1, g_y1, g_x2, g_y2;

    g_x1 = (int)(rcSrc.x1);
    g_y1 = (int)(rcSrc.y1);
    g_x2 = (int)(rcSrc.x2);
    g_y2 = (int)(rcSrc.y2);

    ploygon[0] = rcDest.left;
    ploygon[1] = rcDest.top;
    ploygon[2] = rcDest.right;
    ploygon[3] = rcDest.top;
    ploygon[4] = rcDest.right;
    ploygon[5] = rcDest.bottom;
    ploygon[6] = rcDest.left;
    ploygon[7] = rcDest.bottom;

    agg::trans_bilinear tr(ploygon, g_x1, g_y1, g_x2, g_y2);
    if(tr.is_valid()) {
        typedef agg::span_interpolator_linear<agg::trans_bilinear> interpolator_type;
        interpolator_type interpolator(tr);

        typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
        interpolator_type> span_gen_type;
        span_gen_type sg(ia, interpolator, filter);
        agg::render_scanlines_aa(g_rasterizer, g_scanline, rb_pre, sa, sg);
    }
}

void stretch_buff_bilinear_bgr24(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, CRect &rcDest, agg::rect_f &rcSrc) {
    typedef agg::pixfmt_bgr24 pixfmt;
    typedef pixfmt::color_type color_type;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    typedef agg::pixfmt_bgr24_pre pixfmt_pre;
    typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

    agg::scanline_u8 g_scanline;
    agg::rasterizer_scanline_aa<> g_rasterizer;
    pixfmt pixf(bufDest);
    pixfmt_pre pixf_pre(bufDest);
    renderer_base rb(pixf);
    renderer_base_pre rb_pre(pixf_pre);

    renderer_solid r(rb);


    rb.clear(agg::rgba(1, 1, 1));

    g_rasterizer.clip_box(0, 0, bufDest.width(), bufDest.height());
    g_rasterizer.reset();
    g_rasterizer.move_to_d(rcDest.left, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.top);
    g_rasterizer.line_to_d(rcDest.right, rcDest.bottom);
    g_rasterizer.line_to_d(rcDest.left, rcDest.bottom);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    pixfmt pixf_img(bufSrc);

    typedef agg::image_accessor_clone<pixfmt> img_accessor_type;
    img_accessor_type ia(pixf_img);

    double ploygon[8];
    int g_x1, g_y1, g_x2, g_y2;

    g_x1 = (int)(rcSrc.x1);
    g_y1 = (int)(rcSrc.y1);
    g_x2 = (int)(rcSrc.x2);
    g_y2 = (int)(rcSrc.y2);

    ploygon[0] = rcDest.left;
    ploygon[1] = rcDest.top;
    ploygon[2] = rcDest.right;
    ploygon[3] = rcDest.top;
    ploygon[4] = rcDest.right;
    ploygon[5] = rcDest.bottom;
    ploygon[6] = rcDest.left;
    ploygon[7] = rcDest.bottom;

    agg::trans_bilinear tr(ploygon, g_x1, g_y1, g_x2, g_y2);
    if(tr.is_valid()) {
        typedef agg::span_interpolator_linear<agg::trans_bilinear> interpolator_type;
        interpolator_type interpolator(tr);

        typedef agg::span_image_filter_rgb_2x2<img_accessor_type,
        interpolator_type> span_gen_type;
        span_gen_type sg(ia, interpolator, filter);
        agg::render_scanlines_aa(g_rasterizer, g_scanline, rb_pre, sa, sg);
    }
}

bool stretchBltBilinear(RawImageData *pImageDst, CRect &rcDst, RawImageData *pImageSrc, agg::rect_f &rcSrc, BlendPixMode bpm = BPM_BLEND) {
    if (pImageDst->bitCount == pImageSrc->bitCount) {
        agg::rendering_buffer bufDest(pImageDst->buff, pImageDst->width, pImageDst->height, pImageDst->stride);
        agg::rendering_buffer bufSrc(pImageSrc->buff, pImageSrc->width, pImageSrc->height, pImageSrc->stride);
        if (pImageDst->bitCount == 32) {
            stretch_buff_bilinear_bgra32(bufDest, bufSrc, rcDst, rcSrc);
        } else if (pImageDst->bitCount == 24) {
            stretch_buff_bilinear_bgr24(bufDest, bufSrc, rcDst, rcSrc);
        } else {
            return false;
        }

        return true;
    }

    return false;
}

bool bltRawImage(RawImageData *pImageDst, CRect &rcDst, RawImageData *pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc) {
    if (nOpacitySrc == 255) {
        CBuffBitBlter blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;

        return XBltRawImage<blend_rgb, blend_alpha, copy_rgb, copy_alpha, copy_alpha_255, blend_copy_none, multiply_rgb, multiply_alpha>(blter, pImageDst, pImageSrc, bpm);
    } else if (nOpacitySrc == 0) {
        return true;
    }
    {
        CBuffBitBlterOpacity blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;
        blter.nOpacitySrc = nOpacitySrc;

        return XBltRawImage<blend_rgb_opacity, blend_alpha_opacity, copy_rgb_opacity, copy_alpha_opacity, copy_alpha_255_opacity, blend_copy_none_opacity, multiply_rgb_opacity, multiply_alpha_opacity>(blter, pImageDst, pImageSrc, bpm);
    }
}


bool stretchBlt(RawImageData *pImageSrc, RawImageData *pImageDst, CRect &rcDst, agg::rect_f &rcSrc, BlendPixMode bpm, int nOpacitySrc) {
    if (isFlagSet(bpm, BPM_BILINEAR)) {
        return stretchBltBilinear(pImageDst, rcDst, pImageSrc, rcSrc, bpm & ~BPM_BILINEAR);
    }

    if (nOpacitySrc == 255) {
        CBuffStretchBitBlter blter;

        blter.rcDest = rcDst;
        blter.rcSrc = rcSrc;

        return XBltRawImage<blend_rgb, blend_alpha, copy_rgb, copy_alpha, copy_alpha_255, blend_copy_none, multiply_rgb, multiply_alpha>(blter, pImageDst, pImageSrc, bpm);
    } else if (nOpacitySrc == 0) {
        return true;
    } else {
        CBuffStretchBitBlterOpacity blter;

        blter.rcDest = rcDst;
        blter.rcSrc = rcSrc;
        blter.nOpacitySrc = nOpacitySrc;

        return XBltRawImage<blend_rgb_opacity, blend_alpha_opacity, copy_rgb_opacity, copy_alpha_opacity, copy_alpha_255_opacity, blend_copy_none_opacity, multiply_rgb_opacity, multiply_alpha_opacity>(blter, pImageDst, pImageSrc, bpm);
    }
}


//////////////////////////////////////////////////////////////////////

CRawImage::CRawImage(void) {
    m_image = nullptr;
    m_x = 0;
    m_y = 0;
    m_cx = 0;
    m_cy = 0;
}

CRawImage::~CRawImage(void) {
    destroy();
}

void CRawImage::attach(RawImageData *image) {
    if (!image) {
        destroy();
        return;
    }
    m_x = 0;
    m_y = 0;

    m_cx = image->width;
    m_cy = image->height;

    m_image = image;
}

void CRawImage::detach() {
    m_image = nullptr;
}

void CRawImage::destroy() {
    if (m_image) {
        freeRawImage(m_image);
        m_image = nullptr;
    }
}

bool CRawImage::load(cstr_t szFile) {
    destroy();

    RawImageData *image = loadRawImageDataFromFile(szFile);

    if (image) {
        attach(image);
    }

    return image != nullptr;
}

void CRawImage::blt(CRawGraph *canvas, int xDest, int yDest, BlendPixMode bpm) {
    blt(canvas, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm);
}

void CRawImage::blt(CRawImage *pImage, int xDest, int yDest, BlendPixMode bpm, int nOpacitySrc) {
    blt(pImage, xDest, yDest, m_cx, m_cy, m_x, m_y, bpm, nOpacitySrc);
}

bool CRawImage::blt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm) {
    if (!m_image) {
        return false;
    }

    CRect rcDst;
    RawImageData *pImageDst;

    canvas->getMappedClipBoundRect(rcDst);

    xDest = canvas->mapX(xDest);
    yDest = canvas->mapY(yDest);

    // get the clip box to copy
    if (xDest + widthDest < rcDst.right) {
        rcDst.right = xDest + widthDest;
    }
    if (yDest + heightDest < rcDst.bottom) {
        rcDst.bottom = yDest + heightDest;
    }

    if (xDest >= rcDst.left) {
        rcDst.left = xDest;
    } else {
        xSrc += rcDst.left - xDest;
    }
    if (yDest >= rcDst.top) {
        rcDst.top = yDest;
    } else {
        ySrc += rcDst.top - yDest;
    }

    if (xSrc + rcDst.width() > m_x + m_cx) {
        rcDst.right = rcDst.left + m_x + m_cx - xSrc;
    }
    if (ySrc + rcDst.height() > m_y + m_cy) {
        rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;
    }

    if (rcDst.width() <= 0 || rcDst.height() <= 0) {
        return true;
    }

    pImageDst = canvas->getRawBuff();

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    return bltRawImage(pImageDst, rcDst, m_image, xSrc, ySrc, bpm, canvas->getOpacityPainting());
}

void CRawImage::maskBlt(CRawGraph *canvas, int xDest, int yDest, CRawImage *pImgMask, BlendPixMode bpm) {
    maskBlt(canvas, xDest, yDest, m_cx, m_cy, m_x, m_y, pImgMask, bpm);
}

bool CRawImage::maskBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *pImgMask, BlendPixMode bpm) {
    if (!m_image) {
        return false;
    }

    CRect rcDst;
    RawImageData *pImageDst;

    canvas->getMappedClipBoundRect(rcDst);

    xDest = canvas->mapX(xDest);
    yDest = canvas->mapY(yDest);

    // get the clip box to copy
    if (xDest + widthDest < rcDst.right) {
        rcDst.right = xDest + widthDest;
    }
    if (yDest + heightDest < rcDst.bottom) {
        rcDst.bottom = yDest + heightDest;
    }

    // determine the start position of Graph, image, mask.
    if (xDest >= rcDst.left) {
        rcDst.left = xDest;
    } else {
        xSrc += rcDst.left - xDest;
    }
    if (yDest >= rcDst.top) {
        rcDst.top = yDest;
    } else {
        ySrc += rcDst.top - yDest;
    }

    // make sure the right is within the image region.
    int nWidthMax = min(m_cx, pImgMask->width());
    int nHeightMax = min(m_cy, pImgMask->height());

    if (xSrc + rcDst.width() > m_x + nWidthMax) {
        rcDst.right = rcDst.left + m_x + nWidthMax - xSrc;
    }
    if (ySrc + rcDst.height() > m_y + nHeightMax) {
        rcDst.bottom = rcDst.top + m_y + nHeightMax - ySrc;
    }

    if (rcDst.empty()) {
        return true;
    }

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    pImageDst = canvas->getRawBuff();

    return maskBlt(pImageDst, rcDst, xSrc, ySrc, pImgMask->getHandle(), xSrc, ySrc, bpm, canvas->getOpacityPainting());
}

bool CRawImage::blt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc) {
    if (!m_image || !pImage->m_image) {
        return false;
    }

    if (nOpacitySrc == 0) {
        return true;
    }

    CRect rcDst(xDest, yDest, widthDest + xDest, heightDest + yDest);

    if (rcDst.left < pImage->m_x) {
        rcDst.left = pImage->m_x;
    }
    if (rcDst.top < pImage->m_y) {
        rcDst.top = pImage->m_y;
    }

    if (xSrc + widthDest > m_x + m_cx) {
        rcDst.right = rcDst.left + m_x + m_cx - xSrc;
    }
    if (ySrc + heightDest > m_y + m_cy) {
        rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;
    }

    if (rcDst.width() > pImage->m_cx) {
        rcDst.right = rcDst.left + pImage->m_cx;
    }
    if (rcDst.height() > pImage->m_cy) {
        rcDst.bottom = rcDst.top + pImage->m_cy;
    }

    if (rcDst.top >= pImage->m_y + pImage->m_cy) {
        return true;
    }

    if (rcDst.left >= pImage->m_x + pImage->m_cx) {
        return true;
    }

    return bltRawImage(pImage->m_image, rcDst, m_image, xSrc, ySrc, bpm, nOpacitySrc);
}

void CRawImage::stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm) {
    stretchBlt(canvas, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm);
}

bool CRawImage::stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm) {
    if (!m_image) {
        return false;
    }

    CRect rcDst;
    RawImageData *pImageDst;

    if (xSrc >= m_image->width || ySrc >= m_image->height
        || widthDest == 0 || heightDest == 0) {
        return true;
    }

    if (xSrc + widthSrc > m_x + m_cx) {
        widthSrc = m_x + m_cx - xSrc;
    }
    if (ySrc + heightSrc > m_y + m_cy) {
        heightSrc = m_y + m_cy - ySrc;
    }

    float dx, dy;
    agg::rect_f rcSrc;

    dx = (float)widthSrc / widthDest;
    dy = (float)heightSrc / heightDest;

    canvas->getMappedClipBoundRect(rcDst);

    xDest = canvas->mapX(xDest);
    yDest = canvas->mapY(yDest);

    if (xDest > rcDst.left) {
        rcDst.left = xDest;
        rcSrc.x1 = (float)xSrc;
    } else {
        rcSrc.x1 = xSrc + dx * (rcDst.left - xDest);
    }

    if (yDest > rcDst.top) {
        rcDst.top = yDest;
        rcSrc.y1 = (float)ySrc;
    } else {
        rcSrc.y1 = ySrc + dy * (rcDst.top - yDest);
    }

    // get the clip box to copy
    if (xDest + widthDest <= rcDst.right) {
        rcDst.right = xDest + widthDest;
        rcSrc.x2 = (float)(xSrc + widthSrc);
    } else {
        rcSrc.x2 = xSrc + widthSrc - dx * (xDest + widthDest - rcDst.right);
    }

    if (yDest + heightDest <= rcDst.bottom) {
        rcDst.bottom = yDest + heightDest;
        rcSrc.y2 = (float)(ySrc + heightSrc);
    } else {
        rcSrc.y2 = ySrc + heightSrc - dy * (yDest + heightDest - rcDst.bottom);
    }

    if (rcDst.width() <= 0 || rcDst.height() <= 0) {
        return true;
    }

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    pImageDst = canvas->getRawBuff();

    assert(rcSrc.x1 >= xSrc);
    assert(rcSrc.x2 <= xSrc + widthSrc);
    assert(rcSrc.y1 >= ySrc);
    assert(rcSrc.y2 <= ySrc + heightSrc);

    return ::stretchBlt(m_image, pImageDst, rcDst, rcSrc, bpm, canvas->getOpacityPainting());
}

void CRawImage::stretchBlt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, BlendPixMode bpm, int nOpacitySrc) {
    stretchBlt(pImage, xDest, yDest, widthDest, heightDest, m_x, m_y, m_cx, m_cy, bpm, nOpacitySrc);
}


bool CRawImage::stretchBlt(CRawImage *pImage, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm, int nOpacitySrc) {
    if (!m_image || !pImage->m_image) {
        return false;
    }

    CRect rcDst;

    if (xSrc >= m_image->width || ySrc >= m_image->height
        || widthDest == 0 || heightDest == 0) {
        return true;
    }

    if (xSrc + widthSrc > m_x + m_cx) {
        widthSrc = m_x + m_cx - xSrc;
    }
    if (ySrc + heightSrc > m_y + m_cy) {
        heightSrc = m_y + m_cy - ySrc;
    }

    float dx, dy;
    agg::rect_f rcSrc;

    dx = (float)widthSrc / widthDest;
    dy = (float)heightSrc / heightDest;

    rcDst.setLTRB(pImage->m_x, pImage->m_y, pImage->m_x + pImage->m_cx, pImage->m_y + pImage->m_cy);

    if (xDest > rcDst.left) {
        rcDst.left = xDest;
        rcSrc.x1 = (float)xSrc;
    } else {
        rcSrc.x1 = xSrc + dx * (xDest - rcDst.left);
    }

    if (yDest > rcDst.top) {
        rcDst.top = yDest;
        rcSrc.y1 = (float)ySrc;
    } else {
        rcSrc.y1 = ySrc + dy * (rcDst.top - yDest);
    }

    // get the clip box to copy
    if (xDest + widthDest < rcDst.right) {
        rcDst.right = xDest + widthDest;
        rcSrc.x2 = (float)(xSrc + widthSrc);
    } else {
        rcSrc.x2 = xSrc + widthSrc - dx * (xDest + widthDest - rcDst.right);
    }

    if (yDest + heightDest < rcDst.bottom) {
        rcDst.bottom = yDest + heightDest;
        rcSrc.y2 = (float)(ySrc + heightSrc);
    } else {
        rcSrc.y2 = ySrc + heightSrc - dy * (yDest + heightDest - rcDst.bottom);
    }

    if (rcDst.width() <= 0 || rcDst.height() <= 0) {
        return true;
    }

    if (rcDst.top >= pImage->m_y + pImage->m_cy) {
        return true;
    }

    if (rcDst.left >= pImage->m_x + pImage->m_cx) {
        return true;
    }

    return ::stretchBlt(m_image, pImage->m_image, rcDst, rcSrc, bpm, nOpacitySrc);
}

// Mask blt
bool CRawImage::maskBlt(RawImageData *pImageDst, CRect &rcDst, int xSrc, int ySrc, RawImageData *pImageMask, int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc) {
    if (nOpacitySrc == 255) {
        CBuffMaskBlter blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;
        blter.xMask = xMask;
        blter.yMask = yMask;
        blter.pImageMask = pImageMask;

        return XBltRawImage<mask_blend_rgb, mask_blend_alpha, mask_copy_rgb, mask_copy_alpha, mask_copy_alpha_255, mask_blend_copy_none, mask_multiply_rgb, mask_multiply_alpha>(blter, pImageDst, m_image, bpm);
    } else if (nOpacitySrc == 0) {
        return true;
    } else {
        CBuffMaskBlterOpacity blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;
        blter.xMask = xMask;
        blter.yMask = yMask;
        blter.pImageMask = pImageMask;
        blter.nOpacitySrc = nOpacitySrc;

        return XBltRawImage<mask_blend_rgb_opacity, mask_blend_alpha_opacity, mask_copy_rgb_opacity, mask_copy_alpha_opacity, mask_copy_alpha_255_opacity, mask_blend_copy_none_opacity, mask_multiply_rgb_opacity, mask_multiply_alpha_opacity>(blter, pImageDst, m_image, bpm);
    }
}

bool CRawImage::getOrginalSize(int &nWidth, int &nHeight) {
    if (!m_image) {
        return false;
    }

    nWidth = m_cx = m_image->width;
    nHeight = m_cy = m_image->height;

    return true;
}
