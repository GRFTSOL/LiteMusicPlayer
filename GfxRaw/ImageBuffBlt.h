#ifndef _IMAGE_BUFF_BLT_
#define _IMAGE_BUFF_BLT_

#pragma once

#include "../third-parties/Agg/include/agg_scanline_u.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_span_allocator.h"
#include "../third-parties/Agg/include/agg_span_interpolator_linear.h"
#include "../third-parties/Agg/include/agg_span_interpolator_trans.h"
#include "../third-parties/Agg/include/agg_span_subdiv_adaptor.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_image_accessors.h"
#include "../third-parties/Agg/include/agg_span_image_filter_rgba.h"
#include "../third-parties/Agg/include/agg_span_image_filter_rgb.h"
#include "../third-parties/Agg/include/agg_trans_bilinear.h"


template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, CRect &rcDest, int xSrc, int ySrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
    uint8_t *rowSrc, *rowDest, *pixSrc, *pixDest;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);
    rowSrc = pixfSrc.pix_ptr(xSrc, ySrc);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        pixDest = rowDest;
        pixSrc = rowSrc;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            PixRGBBlender::blend(pixDest, pixSrc);
            PixAlphaBlender::blend(pixDest, pixSrc);

            pixDest += pixfmtDst::pix_width;
            pixSrc += pixfmtSrc::pix_width;
        }
        rowDest += pixfDest.stride();
        rowSrc += pixfSrc.stride();
    }
}


template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
void blt_buff_opacity(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, CRect &rcDest, int xSrc, int ySrc, int nOpacitySrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
    uint8_t *rowSrc, *rowDest, *pixSrc, *pixDest;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);
    rowSrc = pixfSrc.pix_ptr(xSrc, ySrc);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        pixDest = rowDest;
        pixSrc = rowSrc;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            PixRGBBlender::blend(pixDest, pixSrc, nOpacitySrc);
            PixAlphaBlender::blend(pixDest, pixSrc, nOpacitySrc);

            pixDest += pixfmtDst::pix_width;
            pixSrc += pixfmtSrc::pix_width;
        }
        rowDest += pixfDest.stride();
        rowSrc += pixfSrc.stride();
    }
}


//
// 双线性内插值：
//　对于一个目的像素，设置坐标通过反向变换得到的浮点坐标为(i+u,j+v)，其中i、j均为非负整数，u、v为[0,1)区间的浮点数，则这个像素得值 f(i+u,j+v) 可由原图像中坐标为 (i,j)、(i+1,j)、(i,j+1)、(i+1,j+1)所对应的周围四个像素的值决定，即：
//    f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
//
template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
void stretch_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, CRect &rcDest, agg::rect_f &rcSrc, PixRGBBlender &, PixAlphaBlender &) {
    float dx, dy, xSrc, ySrc;
    uint8_t *rowSrc, *rowDest, *pixSrc, *pixDest;

    dx = (float)(rcSrc.x2 - rcSrc.x1) / rcDest.width();
    dy = (float)(rcSrc.y2 - rcSrc.y1) / rcDest.height();
    ySrc = rcSrc.y1;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        xSrc = rcSrc.x1;
        rowSrc = pixfSrc.row_ptr((int)ySrc);
        pixDest = rowDest;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            pixSrc = rowSrc + ((int)xSrc) * pixfmtSrc::pix_width;
            PixRGBBlender::blend(pixDest, pixSrc);
            PixAlphaBlender::blend(pixDest, pixSrc);

            xSrc += dx;
            pixDest += pixfmtDst::pix_width;
        }
        ySrc += dy;
        rowDest += pixfDest.stride();
    }
}

template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
void stretch_buff_opacity(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, CRect &rcDest, agg::rect_f &rcSrc, int nOpacitySrc, PixRGBBlender &, PixAlphaBlender &) {
    float dx, dy, xSrc, ySrc;
    uint8_t *rowSrc, *rowDest, *pixSrc, *pixDest;

    dx = (float)(rcSrc.x2 - rcSrc.x1) / rcDest.width();
    dy = (float)(rcSrc.y2 - rcSrc.y1) / rcDest.height();
    ySrc = rcSrc.y1;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        xSrc = rcSrc.x1;
        rowSrc = pixfSrc.row_ptr((int)ySrc);
        pixDest = rowDest;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            pixSrc = rowSrc + ((int)xSrc) * pixfmtSrc::pix_width;
            PixRGBBlender::blend(pixDest, pixSrc, nOpacitySrc);
            PixAlphaBlender::blend(pixDest, pixSrc, nOpacitySrc);

            xSrc += dx;
            pixDest += pixfmtDst::pix_width;
        }
        ySrc += dy;
        rowDest += pixfDest.stride();
    }
}


template<class pixfmtDst, class pixfmtSrc, class pixfmtMask, class PixRGBBlender, class PixAlphaBlender>
void mask_blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, pixfmtMask &pixfMask, CRect &rcDest, int xSrc, int ySrc, int xMask, int yMask, PixRGBBlender &, PixAlphaBlender &) {
    uint8_t *rowSrc, *rowDest, *rowMask, *pixSrc, *pixDest, *pixMask;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);
    rowSrc = pixfSrc.pix_ptr(xSrc, ySrc);
    rowMask = pixfMask.pix_ptr(xMask, yMask);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        pixDest = rowDest;
        pixSrc = rowSrc;
        pixMask = rowMask;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            if (pixMask[3] != 0) {
                PixRGBBlender::blend(pixDest, pixSrc, pixMask);
                PixAlphaBlender::blend(pixDest, pixSrc, pixMask);
            }

            pixDest += pixfmtDst::pix_width;
            pixSrc += pixfmtSrc::pix_width;
            pixMask += pixfmtMask::pix_width;
        }
        rowDest += pixfDest.stride();
        rowSrc += pixfSrc.stride();
        rowMask += pixfMask.stride();
    }
}


template<class pixfmtDst, class pixfmtSrc, class pixfmtMask, class PixRGBBlender, class PixAlphaBlender>
void mask_blt_buff_opacity(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, pixfmtMask &pixfMask, CRect &rcDest, int xSrc, int ySrc, int xMask, int yMask, int nOpacitySrc, PixRGBBlender &, PixAlphaBlender &) {
    uint8_t *rowSrc, *rowDest, *rowMask, *pixSrc, *pixDest, *pixMask;

    rowDest = pixfDest.pix_ptr(rcDest.left, rcDest.top);
    rowSrc = pixfSrc.pix_ptr(xSrc, ySrc);
    rowMask = pixfMask.pix_ptr(xMask, yMask);

    for (int y = rcDest.top; y < rcDest.bottom; y++) {
        pixDest = rowDest;
        pixSrc = rowSrc;
        pixMask = rowMask;
        for (int x = rcDest.left; x < rcDest.right; x++) {
            if (pixMask[3] != 0) {
                PixRGBBlender::blend(pixDest, pixSrc, pixMask, nOpacitySrc);
                PixAlphaBlender::blend(pixDest, pixSrc, pixMask, nOpacitySrc);
            }

            pixDest += pixfmtDst::pix_width;
            pixSrc += pixfmtSrc::pix_width;
            pixMask += pixfmtMask::pix_width;
        }
        rowDest += pixfDest.stride();
        rowSrc += pixfSrc.stride();
        rowMask += pixfMask.stride();
    }
}

#define BlendAlpha(a1, a2)  ((uint8_t)(((a1) + (a2)) - (((a1) * (a2) + 0xFF) >> 8)))

class blend_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        uint8_t alpha = src[3]; // 256 - 0 = 0 for uint8_t.
        //         dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        //         dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        //         dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
        dst[0] = (uint8_t)(((dst[0] * (256 - alpha)) >> 8) + src[0]);
        dst[1] = (uint8_t)(((dst[1] * (256 - alpha)) >> 8) + src[1]);
        dst[2] = (uint8_t)(((dst[2] * (256 - alpha)) >> 8) + src[2]);
    }
};

class blend_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        // uint8_t    alpha = src[3];

        // dst[3] = (uint8_t)((src[3] + dst[3]) - ((src[3] * dst[3] + 0xFF) >> 8));
        dst[3] = BlendAlpha(dst[3], src[3]);
    }
};

class blend_copy_none {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
    }
};


class copy_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }
};


class copy_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        dst[3] = src[3];
    }
};


class copy_alpha_255 {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        dst[3] = 255;
    }
};

class multiply_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        uint8_t alpha = src[3];
        dst[0] = (uint8_t)((dst[0] * alpha) / 255);
        dst[1] = (uint8_t)((dst[1] * alpha) / 255);
        dst[2] = (uint8_t)((dst[2] * alpha) / 255);
    }
};

class multiply_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src) {
        dst[3] = (uint8_t)((dst[3] * src[3]) / 255);
    }
};

class CBuffBitBlter {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        ::blt_buff(pixfDest, pixfSrc, rcDest, xSrc, ySrc, pixRGBBlender, pixAlphaBlender);
    }

    CRect                       rcDest;
    int                         xSrc, ySrc;

};

class CBuffStretchBitBlter {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        ::stretch_buff(pixfDest, pixfSrc, rcDest, rcSrc, pixRGBBlender, pixAlphaBlender);
    }

    CRect                       rcDest;
    agg::rect_f                 rcSrc;

};

////////////////////////////////////////////////////////////////////////////////
// Blend pixel with opacity options

class blend_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        uint8_t alpha = uint8_t(src[3] * nOpacitySrc / 255);
        dst[0] = (uint8_t)((dst[0] * (256 - alpha) + src[0] * nOpacitySrc) >> 8);
        dst[1] = (uint8_t)((dst[1] * (256 - alpha) + src[1] * nOpacitySrc) >> 8);
        dst[2] = (uint8_t)((dst[2] * (256 - alpha) + src[2] * nOpacitySrc) >> 8);
        //         dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        //         dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        //         dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
    }
};

class blend_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        uint8_t alpha = uint8_t(src[3] * nOpacitySrc / 255);
        // dst[3] = (uint8_t)((src[3] + dst[3]) - ((src[3] * dst[3] + 0xFF) >> 8));
        dst[3] = BlendAlpha(dst[3], alpha);
    }
};

class blend_copy_none_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
    }
};


class copy_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        dst[0] = (uint8_t)((src[0] * nOpacitySrc >> 8) + (dst[0] * (256-nOpacitySrc) >> 8));
        dst[1] = (uint8_t)((src[1] * nOpacitySrc >> 8) + (dst[1] * (256-nOpacitySrc) >> 8));
        dst[2] = (uint8_t)((src[2] * nOpacitySrc >> 8) + (dst[2] * (256-nOpacitySrc) >> 8));
        //        dst[0] = (uint8_t)(src[0] * nOpacitySrc >> 8);
        //        dst[1] = (uint8_t)(src[1] * nOpacitySrc >> 8);
        //        dst[2] = (uint8_t)(src[2] * nOpacitySrc >> 8);
        //         dst[0] = (uint8_t)(((src[0] - dst[0]) * nOpacitySrc + (dst[0] << 8)) >> 8);
        //         dst[1] = (uint8_t)(((src[1] - dst[1]) * nOpacitySrc + (dst[1] << 8)) >> 8);
        //         dst[2] = (uint8_t)(((src[2] - dst[2]) * nOpacitySrc + (dst[2] << 8)) >> 8);
    }
};


class copy_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        // uint8_t    alpha = uint8_t(src[3] * (255 - (255 - src[3]) * nOpacitySrc / 255) / 255);
        // dst[3] = (uint8_t)((src[3] + dst[3]) - ((src[3] * dst[3] + 0xFF) >> 8));
        // dst[3] = uint8_t(src[3] * nOpacitySrc / 255);
        dst[3] = uint8_t(src[3] * nOpacitySrc / 255 + (dst[3] * (256-nOpacitySrc) >> 8));
    }
};


class copy_alpha_255_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        dst[3] = 255;
    }
};

class multiply_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        // uint8_t    alpha = uint8_t(src[3] * nOpacitySrc / 255);
        uint8_t alpha = uint8_t(255 - (255 - src[3]) * nOpacitySrc / 255);
        dst[0] = (uint8_t)((dst[0] * alpha) / 255);
        dst[1] = (uint8_t)((dst[1] * alpha) / 255);
        dst[2] = (uint8_t)((dst[2] * alpha) / 255);
    }
};

class multiply_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, int nOpacitySrc) {
        uint8_t alpha = uint8_t(255 - (255 - src[3]) * nOpacitySrc / 255);
        dst[3] = (uint8_t)((dst[3] * alpha) / 255);
    }
};

class CBuffBitBlterOpacity {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        ::blt_buff_opacity(pixfDest, pixfSrc, rcDest, xSrc, ySrc, nOpacitySrc, pixRGBBlender, pixAlphaBlender);
    }

    CRect                       rcDest;
    int                         xSrc, ySrc;
    int                         nOpacitySrc;

};


class CBuffStretchBitBlterOpacity {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        ::stretch_buff_opacity(pixfDest, pixfSrc, rcDest, rcSrc, nOpacitySrc, pixRGBBlender, pixAlphaBlender);
    }

    CRect                       rcDest;
    agg::rect_f                 rcSrc;
    int                         nOpacitySrc;

};


class CBuffMaskBlter {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        agg::rendering_buffer rbufMask(imageMask->buff, imageMask->width, imageMask->height, imageMask->stride);
        typedef agg::pixfmt_rgba32 pixfmt32;
        pixfmt32 pixfMask(rbufMask);

        ::mask_blt_buff(pixfDest, pixfSrc, pixfMask, rcDest, xSrc, ySrc, xMask, yMask, pixRGBBlender, pixAlphaBlender);
    }

    RawImageData                *imageMask;
    CRect                       rcDest;
    int                         xSrc, ySrc, xMask, yMask;

};

class CBuffMaskBlterOpacity {
public:
    template<class pixfmtDst, class pixfmtSrc, class PixRGBBlender, class PixAlphaBlender>
    inline void blt_buff(pixfmtDst &pixfDest, pixfmtSrc &pixfSrc, PixRGBBlender &pixRGBBlender, PixAlphaBlender &pixAlphaBlender) {
        agg::rendering_buffer rbufMask(imageMask->buff, imageMask->width, imageMask->height, imageMask->stride);
        typedef agg::pixfmt_rgba32 pixfmt32;
        pixfmt32 pixfMask(rbufMask);

        ::mask_blt_buff_opacity(pixfDest, pixfSrc, pixfMask, rcDest, xSrc, ySrc, xMask, yMask, nOpacitySrc, pixRGBBlender, pixAlphaBlender);
    }

    RawImageData                *imageMask;
    CRect                       rcDest;
    int                         xSrc, ySrc, xMask, yMask;
    int                         nOpacitySrc;

};

template<class _rgb_blender, class _alpha_blender, class _blend_copy_none, class _BuffBitBlter, class _pixFmtDst, class _pixFmtSrc>
inline void selectBlenderBlt(_BuffBitBlter &blter, _pixFmtDst &pixfDst, _pixFmtSrc &pixfSrc, bool bRgb, bool bAlpha);

template<class _rgb_blender, class _alpha_blender, class _blend_copy_none, class _BuffBitBlter, class _pixFmtDst, class _pixFmtSrc>
inline void selectBlenderBlt(_BuffBitBlter &blter, _pixFmtDst &pixfDst, _pixFmtSrc &pixfSrc, bool bRgb, bool bAlpha) {
    if (bRgb) {
        _rgb_blender rgbBlender;
        if (bAlpha) {
            _alpha_blender alphaBlender;
            blter.blt_buff(pixfDst, pixfSrc, rgbBlender, alphaBlender);
        } else {
            _blend_copy_none alphaBlender;
            blter.blt_buff(pixfDst, pixfSrc, rgbBlender, alphaBlender);
        }
    } else {
        _blend_copy_none rgbBlender;
        if (bAlpha) {
            _alpha_blender alphaBlender;
            blter.blt_buff(pixfDst, pixfSrc, rgbBlender, alphaBlender);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// blend pixel with mask

class mask_blend_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        uint8_t alpha = uint8_t(src[3] * mask[3] / 255);
        dst[0] = (uint8_t)((dst[0] * (256 - alpha) + src[0] * mask[3]) >> 8);
        dst[1] = (uint8_t)((dst[1] * (256 - alpha) + src[1] * mask[3]) >> 8);
        dst[2] = (uint8_t)((dst[2] * (256 - alpha) + src[2] * mask[3]) >> 8);
        //        uint8_t    alpha = mask[3];
        //         dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        //         dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        //         dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
    }
};


class mask_blend_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        // int        alpha = src[3] * mask[3] / 255;
        dst[3] = (uint8_t)((src[3] + dst[3]) - ((src[3] * dst[3] + 0xFF) >> 8));
    }
};


class mask_copy_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        uint8_t alpha = mask[3];
        dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
    }
};

class mask_copy_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        uint8_t alpha = mask[3];
        dst[3] = (uint8_t)(((src[3] - dst[3]) * alpha + (dst[3] << 8)) >> 8);
    }
};

class mask_copy_alpha_255 {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        dst[3] = 255 * mask[3] / 255;
    }
};


class mask_blend_copy_none {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
    }
};


class mask_multiply_rgb {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        int alpha = mask[3] * src[3] / 255;
        dst[0] = (uint8_t)((dst[0] * alpha) / 255);
        dst[1] = (uint8_t)((dst[1] * alpha) / 255);
        dst[2] = (uint8_t)((dst[2] * alpha) / 255);
    }
};

class mask_multiply_alpha {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask) {
        dst[3] = (uint8_t)((dst[3] * src[3] * mask[3]) / 255 / 255);
    }
};


//////////////////////////////////////////////////////////////////////////
// blend pixel with mask and opacity

class mask_blend_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint32_t m = uint8_t(mask[3] * nOpacitySrc / 255);
        uint8_t alpha = uint8_t(src[3] * m / 255);
        dst[0] = (uint8_t)((dst[0] * (256 - alpha) + src[0] * m) >> 8);
        dst[1] = (uint8_t)((dst[1] * (256 - alpha) + src[1] * m) >> 8);
        dst[2] = (uint8_t)((dst[2] * (256 - alpha) + src[2] * m) >> 8);
        //         uint8_t    alpha = uint8_t(mask[3] * nOpacitySrc / 255);
        //         dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        //         dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        //         dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
    }
};


class mask_blend_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint32_t m = uint8_t(mask[3] * nOpacitySrc / 255);
        uint8_t alpha = uint8_t(src[3] * m / 255);
        dst[3] = BlendAlpha(dst[3], alpha);
    }
};


class mask_copy_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint8_t alpha = uint8_t(mask[3] * nOpacitySrc / 255);
        dst[0] = (uint8_t)(((src[0] - dst[0]) * alpha + (dst[0] << 8)) >> 8);
        dst[1] = (uint8_t)(((src[1] - dst[1]) * alpha + (dst[1] << 8)) >> 8);
        dst[2] = (uint8_t)(((src[2] - dst[2]) * alpha + (dst[2] << 8)) >> 8);
    }
};

class mask_copy_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint8_t alpha = uint8_t(mask[3] * nOpacitySrc / 255);
        dst[3] = (uint8_t)(((src[3] - dst[3]) * alpha + (dst[3] << 8)) >> 8);
    }
};

class mask_copy_alpha_255_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        dst[3] = 255 * mask[3] / 255;
    }
};


class mask_blend_copy_none_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
    }
};


class mask_multiply_rgb_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint8_t alpha = uint8_t(255 - (255 - src[3]) * nOpacitySrc / 255);
        alpha = mask[3] * alpha / 255;
        dst[0] = (uint8_t)((dst[0] * alpha) / 255);
        dst[1] = (uint8_t)((dst[1] * alpha) / 255);
        dst[2] = (uint8_t)((dst[2] * alpha) / 255);
    }
};

class mask_multiply_alpha_opacity {
public:
    static inline void blend(uint8_t *dst, uint8_t *src, uint8_t *mask, int nOpacitySrc) {
        uint8_t alpha = uint8_t(255 - (255 - src[3]) * nOpacitySrc / 255);
        dst[3] = (uint8_t)((dst[3] * alpha * mask[3]) / 255 / 255);
    }
};

//////////////////////////////////////////////////////////////////////////
// Common way to blt raw image

template<class _blend_rgb, class _blend_alpha, class _copy_rgb, class _copy_alpha, class _copy_alpha_255, class _blend_copy_none, class _multiply_rgb, class _multiply_alpha, class _BuffBlter>
bool XBltRawImage(_BuffBlter &blter, RawImageData *pImageDst, RawImageData *pImageSrc, BlendPixMode bpm) {
    assert(pImageSrc);
    assert(pImageDst);

    typedef agg::pixfmt_rgba32 pixfmt32;
    typedef agg::pixfmt_rgb24 pixfmt24;

    agg::rendering_buffer rbufDest(pImageDst->buff, pImageDst->width, pImageDst->height, pImageDst->stride);
    agg::rendering_buffer rbufSrc(pImageSrc->buff, pImageSrc->width, pImageSrc->height, pImageSrc->stride);
    PixFormat pfDst = pImageDst->pixFormat, pfSrc = pImageSrc->pixFormat;

    if (isFlagSet(bpm, BPM_OP_BLEND) && pfSrc == PF_RGBA32) {
        //
        // Src is 32 bit, Blend rgb and alpha
        //
        pixfmt32 pixfSrc(rbufSrc);

        if (pfDst == PF_RGB24) {
            pixfmt24 pixfDst(rbufDest);
            selectBlenderBlt<_blend_rgb, _blend_copy_none, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
        } else {
            pixfmt32 pixfDst(rbufDest);

            selectBlenderBlt<_blend_rgb, _blend_alpha, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
        }
    } else if (isFlagSet(bpm, BPM_OP_MULTIPLY) && pfSrc == PF_RGBA32) {
        //
        // Src is 32 bit, multiply rgb and alpha
        //
        pixfmt32 pixfSrc(rbufSrc);

        if (pfDst == PF_RGB24) {
            pixfmt24 pixfDst(rbufDest);
            selectBlenderBlt<_multiply_rgb, _blend_copy_none, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
        } else {
            pixfmt32 pixfDst(rbufDest);

            selectBlenderBlt<_multiply_rgb, _multiply_alpha, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
        }
    } else {
        //
        // Copy pixel or alpha
        //
        if (pfSrc == PF_RGB24) {
            pixfmt24 pixfSrc(rbufSrc);

            if (pfDst == PF_RGB24) {
                pixfmt24 pixfDst(rbufDest);
                selectBlenderBlt<_copy_rgb, _blend_copy_none, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), false);
            } else {
                pixfmt32 pixfDst(rbufDest);

                selectBlenderBlt<_copy_rgb, _copy_alpha_255, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
            }
        } else if (pfSrc == PF_RGBA32) {
            pixfmt32 pixfSrc(rbufSrc);

            if (pfDst == PF_RGB24) {
                pixfmt24 pixfDst(rbufDest);
                selectBlenderBlt<_copy_rgb, _blend_copy_none, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), false);
            } else {
                pixfmt32 pixfDst(rbufDest);

                selectBlenderBlt<_copy_rgb, _copy_alpha, _blend_copy_none>(blter, pixfDst, pixfSrc, isFlagSet(bpm, BPM_CHANNEL_RGB), isFlagSet(bpm, BPM_CHANNEL_ALPHA));
            }
        }
    }

    return true;
}


#endif // _IMAGE_BUFF_BLT_
