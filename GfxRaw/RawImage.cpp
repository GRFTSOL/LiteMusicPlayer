#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"

#include "GfxRaw.h"
#include "RawImage.h"

#include "ImageBuffBlt.h"


bool maskBltRawImage(RawImageData *pImageDst, const CRect &rcDst, int xSrc, int ySrc,
    RawImageData *imageSrc, RawImageData *imageMask,
    int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc);

void scale(CRawGraph *canvas, agg::rect_f &r) {
    r.x1 = canvas->scale(r.x1);
    r.x2 = canvas->scale(r.x2);
    r.y1 = canvas->scale(r.y1);
    r.y2 = canvas->scale(r.y2);
}

void stretch_buff_bilinear_bgra32(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, const CRect &rcDest, const agg::rect_f &rcSrc) {
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

void stretch_buff_bilinear_bgr24(agg::rendering_buffer &bufDest, agg::rendering_buffer &bufSrc, const CRect &rcDest, const agg::rect_f &rcSrc) {
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

bool stretchBltBilinear(RawImageData *pImageDst, const CRect &rcDst, RawImageData *pImageSrc, const agg::rect_f &rcSrc, BlendPixMode bpm = BPM_BLEND) {
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

bool bltRawImage(RawImageData *pImageDst, const CRect &rcDst, RawImageData *pImageSrc, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc) {
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


bool stretchBltRawImage(RawImageData *pImageSrc, RawImageData *pImageDst, const CRect &rcDst, const agg::rect_f &rcSrc, BlendPixMode bpm, int nOpacitySrc) {
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

RawImageDataPtr createScaledRawImageData(RawImageData *src, float scale) {
    RawImageDataPtr newImage = make_shared<RawImageData>();

    int width = src->width * scale;
    int height = src->height * scale;
    newImage->create(width, height, src->bitCount);

    agg::rect_f rcSrc(0, 0, src->width, src->height);
    stretchBltRawImage(src, newImage.get(), CRect(0, 0, width, height), rcSrc, BPM_COPY, 255);

    return newImage;
}

//////////////////////////////////////////////////////////////////////

CRawImage::CRawImage(void) {
    m_x = 0;
    m_y = 0;
    m_cx = 0;
    m_cy = 0;
}

CRawImage::CRawImage(const RawImageDataPtr &image) {
    assert(image);
    attach(image);
}

CRawImage::~CRawImage(void) {
}

void CRawImage::attach(const RawImageDataPtr &image) {
    if (m_image) {
        detach();
    }

    if (!image) {
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

bool CRawImage::load(cstr_t szFile) {
    m_image = loadRawImageDataFromFile(szFile);
    return m_image != nullptr;
}

bool CRawImage::blt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm) {
    CRect rcDst;
    canvas->getClipBoundBox(rcDst);

    // get the clip box to copy
    rcDst.right = min(rcDst.right, xDest + widthDest);
    rcDst.bottom = min(rcDst.bottom, yDest + heightDest);

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

    if (xSrc + rcDst.width() > m_x + m_cx) {
        rcDst.right = rcDst.left + m_x + m_cx - xSrc;
    }
    if (ySrc + rcDst.height() > m_y + m_cy) {
        rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;
    }

    if (rcDst.empty()) {
        return true;
    }

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    // 需要进行像素绘制，转换为内存坐标
    rcDst = canvas->scale(rcDst);
    xSrc = canvas->scale(xSrc);
    ySrc = canvas->scale(ySrc);

    return bltRawImage(canvas->getRawBuff(), rcDst,
       getRawImageData(canvas->getScaleFactor()).get(),
       xSrc, ySrc, bpm, canvas->getOpacityPainting());
}

bool CRawImage::maskBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, CRawImage *imgMask, BlendPixMode bpm)
{
    CRect rcDst;
    canvas->getClipBoundBox(rcDst);

    // get the clip box to copy
    rcDst.right = min(rcDst.right, xDest + widthDest);
    rcDst.bottom = min(rcDst.bottom, yDest + heightDest);

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
    int widthMin = min(m_cx, imgMask->width());
    int heightMin = min(m_cy, imgMask->height());

    if (xSrc + rcDst.width() > m_x + widthMin) {
        rcDst.right = rcDst.left + m_x + widthMin - xSrc;
    }
    if (ySrc + rcDst.height() > m_y + heightMin) {
        rcDst.bottom = rcDst.top + m_y + heightMin - ySrc;
    }

    if (rcDst.empty()) {
        return true;
    }

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    // 需要进行像素绘制，转换为内存坐标
    rcDst = canvas->scale(rcDst);
    xSrc = canvas->scale(xSrc);
    ySrc = canvas->scale(ySrc);

    return maskBltRawImage(canvas->getRawBuff(), rcDst, xSrc, ySrc,
       getRawImageData(canvas->getScaleFactor()).get(),
       imgMask->getRawImageData(canvas->getScaleFactor()).get(),
       xSrc, ySrc, bpm, canvas->getOpacityPainting());
}

bool CRawImage::blt(CRawImage *image, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, BlendPixMode bpm, int nOpacitySrc)
{
    if (!m_image || !image->m_image) {
        return false;
    }

    if (nOpacitySrc == 0) {
        return true;
    }

    CRect rcDst(xDest, yDest, widthDest + xDest, heightDest + yDest);

    if (rcDst.left < image->m_x) {
        rcDst.left = image->m_x;
    }
    if (rcDst.top < image->m_y) {
        rcDst.top = image->m_y;
    }

    if (xSrc + widthDest > m_x + m_cx) {
        rcDst.right = rcDst.left + m_x + m_cx - xSrc;
    }
    if (ySrc + heightDest > m_y + m_cy) {
        rcDst.bottom = rcDst.top + m_y + m_cy - ySrc;
    }

    if (rcDst.width() > image->m_cx) {
        rcDst.right = rcDst.left + image->m_cx;
    }
    if (rcDst.height() > image->m_cy) {
        rcDst.bottom = rcDst.top + image->m_cy;
    }

    if (rcDst.top >= image->m_y + image->m_cy) {
        return true;
    }

    if (rcDst.left >= image->m_x + image->m_cx) {
        return true;
    }

    return bltRawImage(image->m_image.get(), rcDst, m_image.get(), xSrc, ySrc, bpm, nOpacitySrc);
}

bool CRawImage::stretchBlt(CRawGraph *canvas, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm) {

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

    float dx = (float)widthSrc / widthDest;
    float dy = (float)heightSrc / heightDest;

    CRect rcDst;
    canvas->getClipBoundBox(rcDst);

    agg::rect_f rcSrc;
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

    if (rcDst.empty()) {
        return true;
    }

    if (!canvas->getEnableAlphaChannel()) {
        bpm &= ~BPM_CHANNEL_ALPHA;
    }

    assert(rcSrc.x1 >= xSrc);
    assert(rcSrc.x2 <= xSrc + widthSrc);
    assert(rcSrc.y1 >= ySrc);
    assert(rcSrc.y2 <= ySrc + heightSrc);

    // 需要进行像素绘制，转换为内存坐标
    rcDst = canvas->scale(rcDst);
    scale(canvas, rcSrc);

    return stretchBltRawImage(getRawImageData(canvas->getScaleFactor()).get(),
                        canvas->getRawBuff(), rcDst, rcSrc, bpm, canvas->getOpacityPainting());
}

bool CRawImage::stretchBlt(CRawImage *image, int xDest, int yDest, int widthDest, int heightDest, int xSrc, int ySrc, int widthSrc, int heightSrc, BlendPixMode bpm, int nOpacitySrc) {
    if (!m_image || !image->m_image) {
        return false;
    }

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


    float dx = (float)widthSrc / widthDest;
    float dy = (float)heightSrc / heightDest;
    CRect rcDst(image->m_x, image->m_y, image->m_x + image->m_cx, image->m_y + image->m_cy);
    agg::rect_f rcSrc;

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

    if (rcDst.top >= image->m_y + image->m_cy) {
        return true;
    }

    if (rcDst.left >= image->m_x + image->m_cx) {
        return true;
    }

    return ::stretchBltRawImage(m_image.get(), image->m_image.get(), rcDst, rcSrc, bpm, nOpacitySrc);
}

// Mask blt
bool maskBltRawImage(RawImageData *pImageDst, const CRect &rcDst, int xSrc, int ySrc, RawImageData *imageSrc, RawImageData *imageMask, int xMask, int yMask, BlendPixMode bpm, int nOpacitySrc)
{
    if (nOpacitySrc == 255) {
        CBuffMaskBlter blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;
        blter.xMask = xMask;
        blter.yMask = yMask;
        blter.imageMask = imageMask;

        return XBltRawImage<mask_blend_rgb, mask_blend_alpha, mask_copy_rgb, mask_copy_alpha, mask_copy_alpha_255, mask_blend_copy_none, mask_multiply_rgb, mask_multiply_alpha>(blter, pImageDst, imageSrc, bpm);
    } else if (nOpacitySrc == 0) {
        return true;
    } else {
        CBuffMaskBlterOpacity blter;

        blter.rcDest = rcDst;
        blter.xSrc = xSrc;
        blter.ySrc = ySrc;
        blter.xMask = xMask;
        blter.yMask = yMask;
        blter.imageMask = imageMask;
        blter.nOpacitySrc = nOpacitySrc;

        return XBltRawImage<mask_blend_rgb_opacity, mask_blend_alpha_opacity, mask_copy_rgb_opacity, mask_copy_alpha_opacity, mask_copy_alpha_255_opacity, mask_blend_copy_none_opacity, mask_multiply_rgb_opacity, mask_multiply_alpha_opacity>(blter, pImageDst, imageSrc, bpm);
    }
}

void CRawImage::xScaleBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int xExtendStart, int xExtendEnd, bool bTile, BlendPixMode bpm) {
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

void CRawImage::xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
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

void CRawImage::xTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm) {
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

void CRawImage::yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
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

void CRawImage::yTileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, int nImageX, int nImageY, int nImageCx, int nImageCy, BlendPixMode bpm) {
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

void CRawImage::tileBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    CDrawImageFun fun(canvas, this, bpm);

    tileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

void CRawImage::tileMaskBlt(CRawGraph *canvas, int xDest, int yDest, int nWidthDest, int nHeightDest, CRawImage *imageMask, BlendPixMode bpm) {
    CDrawImageFunMask fun(canvas, this, imageMask, bpm);
    tileBltT(xDest, yDest, nWidthDest, nHeightDest, m_x, m_y, m_cx, m_cy, fun);
}

void CRawImage::tileBltEx(CRawGraph *canvas, int xOrg, int yOrg, int xDest, int yDest, int nWidthDest, int nHeightDest, BlendPixMode bpm) {
    CDrawImageFun fun(canvas, this, bpm);

    tileBltExT(canvas, xOrg, yOrg, xDest, yDest, nWidthDest, nHeightDest, fun);
}

bool CRawImage::isPixelTransparent(CPoint pt) const {
    RGBQUAD pixel = m_image->getPixel(pt.x + m_x, pt.y + m_y);
    return pixel.rgbReserved == 0;
}
