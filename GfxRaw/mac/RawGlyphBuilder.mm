#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include "../../third-parties/Agg/include/agg_scanline_p.h"
#include "../../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"
#include "../RawGlyphSet.hpp"


#define MEM_GRAPH_BITS      32

extern bool g_bAntialiasFontEnabled;

CRawGlyphBuilder::CRawGlyphBuilder() {
}

CRawGlyphBuilder::~CRawGlyphBuilder() {
}

void CRawGlyphBuilder::init(const FontInfoEx &font) {
    if (m_fontMemGraph.isValid() && m_fontInfo.isSame(font)) {
        return;
    }

    m_fontInfo = font;

    m_fontMemGraph.destroy();
    m_fontMemGraph.create(getHeightBitmap() * 2, getHeightBitmap(), nullptr, MEM_GRAPH_BITS);

    CGContextSetTextMatrix(m_fontMemGraph.getHandle(), CGAffineTransformMakeScale(1.0, -1.0));

    m_fontLatin.create(font.nameLatin9.c_str(), font.height, font.italic, font.weight, font.isUnderline);
    m_fontOthers.create(font.nameOthers.c_str(), font.height, font.italic, font.weight, font.isUnderline);
}

int CRawGlyphBuilder::getHeight() const {
    return m_fontInfo.getHeight();
}

void CRawGlyphBuilder::destroy() {
    m_fontMemGraph.destroy();
    m_fontLatin.destroy();
    m_fontOthers.destroy();
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgba32 &bufGraph);

int CRawGlyphBuilder::getHeightBitmap() {
    return m_fontInfo.height + MARGIN_FONT * 2;
}

Glyph *CRawGlyphBuilder::buildGlyph(string &ch) {
    RawImageData *rawData = m_fontMemGraph.getRawBuff();
    agg::rendering_buffer bufGraph(rawData->buff, rawData->width, rawData->height, rawData->stride);

    int nHeightBitmap = getHeightBitmap();

    uint8_t *pRowSrc = bufGraph.row_ptr(0);
    for (int y = 0; y < nHeightBitmap; y++) {
        memset(pRowSrc, 0, bufGraph.stride_abs());
        pRowSrc += bufGraph.stride();
    }

    int textWidth = 0;
    CFStringRef string = CFStringCreateWithBytes(nullptr, (UInt8*)ch.c_str(), ch.size(), kCFStringEncodingUTF8, false);
    if (string) {
        int y = MARGIN_FONT + m_fontInfo.height - m_fontInfo.height / 6;
        if (isAnsiStr(ch.c_str())) {
            textWidth = m_fontLatin.draw(m_fontMemGraph.getHandle(), string, MARGIN_FONT, y);
        } else {
            textWidth = m_fontOthers.draw(m_fontMemGraph.getHandle(), string, MARGIN_FONT, y);
        }

        CFRelease(string);
    } else {
        textWidth = m_fontInfo.height / 2;
    }

    Glyph *glyph = new Glyph;
    glyph->ch = ch;
    glyph->nWidth = (uint8_t)textWidth;

    agg::pixfmt_rgba32 pixf(bufGraph);
    copyBitmpDataFromGraphBuffer(glyph, pixf);

    return glyph;
}
