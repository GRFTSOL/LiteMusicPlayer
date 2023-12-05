#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GdiplusGraphicsLite.h"
#include "RawGlyphBuilder.h"
#include "../RawBmpFont.h"
#include "../RawGlyphSet.hpp"


#define MEM_GRAPH_BITS      24

CRawGlyphBuilder::CRawGlyphBuilder() {
}

CRawGlyphBuilder::~CRawGlyphBuilder() {
    destroy();
}

void CRawGlyphBuilder::init(const FontInfoEx &font) {
    if (m_fontMemGraph.isValid() && m_fontInfo.isSame(font)) {
        return;
    }

    m_fontInfo = font;

    createMemGraph(getHeightBitmap() * 2);
}

int CRawGlyphBuilder::getHeight() const {
    return m_fontInfo.getHeight();
}

void CRawGlyphBuilder::createMemGraph(int nWidthGraph) {
    nWidthGraph = (nWidthGraph + 3 % 4) * 4;
    m_fontMemGraph.destroy();

    m_fontMemGraph.create(nWidthGraph, getHeightBitmap(), NULL, MEM_GRAPH_BITS);

    CGdiplusGraphicsLite *pmemGraph;

    m_fontGraph.attach(m_fontMemGraph.getHandle());

    m_fontGraph.setTextColor(CColor(RGB(255, 255, 255)));
    m_fontGraph.setBgColor(CColor(RGB(0, 0, 0)));

    m_fontLatin = m_fontGraph.createFont(m_fontInfo, true);
    m_fontOthers = m_fontGraph.createFont(m_fontInfo, false);
}

void CRawGlyphBuilder::destroy() {
    m_fontMemGraph.destroy();
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgb24 &bufGraph);

Glyph *CRawGlyphBuilder::buildGlyph(string &ch) {
    utf16string ucs2Ch;
    utf8ToUCS2(ch.c_str(), ch.size(), ucs2Ch);

    if (isAnsiStr(ch.c_str())) {
        m_fontGraph.setFont(m_fontLatin);
    } else {
        m_fontGraph.setFont(m_fontOthers);
    }

    CSize size;
    if (!m_fontGraph.getTextExtentPoint32(ucs2Ch.c_str(), ucs2Ch.size(), &size)) {
        return nullptr;
    }

    if (size.cx > m_fontMemGraph.width() + MARGIN_FONT * 2) {
        createMemGraph(size.cx + getHeightBitmap());
    }

    RawImageData *rawBuff = m_fontMemGraph.getRawBuff();
    agg::rendering_buffer bufGraph(rawBuff->buff, rawBuff->width, rawBuff->height, rawBuff->stride);

    int nHeightBitmap = getHeightBitmap();

    uint8_t *pRowSrc = bufGraph.row_ptr(0);
    for (int y = 0; y < nHeightBitmap; y++) {
        memset(pRowSrc, 0, bufGraph.stride_abs());
        pRowSrc += bufGraph.stride();
    }

    int textWidth = 0;
    if (!m_fontGraph.textOut(MARGIN_FONT, MARGIN_FONT, ucs2Ch.c_str(), ucs2Ch.size())) {
        return nullptr;
    }

    Glyph *glyph = new Glyph;
    glyph->ch = ch;
    glyph->nWidth = (uint8_t)size.cx + 1; // 增加一个像素的字体间距.

    agg::pixfmt_rgb24 pixf(bufGraph);
    copyBitmpDataFromGraphBuffer(glyph, pixf);

    return glyph;
}
