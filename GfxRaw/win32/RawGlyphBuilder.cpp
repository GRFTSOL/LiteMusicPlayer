#include "../../GfxLite/win32/GdiGraphicsLite.h"
#include "../../GfxLite/win32/GdiplusGraphicsLite.h"
#include "../third-parties/Agg/include/agg_scanline_p.h"
#include "../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"


namespace GfxRaw
{

#define MEM_GRAPH_BITS      24

extern bool g_bAntialiasFontEnabled;

CRawGlyphBuilder::CRawGlyphBuilder() {
    m_pmemGraph = nullptr;
}

CRawGlyphBuilder::~CRawGlyphBuilder() {
    destroy();
}

void CRawGlyphBuilder::init(const CFontInfo &font) {
    if (m_pmemGraph && m_font.isSame(font)) {
        return;
    }

    m_font.create(font);

    createMemGraph(getHeightBitmap() * 2);
}


int CRawGlyphBuilder::getHeight() const {
    return m_font.getHeight();
}

void CRawGlyphBuilder::createMemGraph(int nWidthGraph) {
    nWidthGraph = (nWidthGraph + 3 % 4) * 4;
    m_memGraph.destroy();
    if (m_pmemGraph) {
        delete m_pmemGraph;
        m_pmemGraph = nullptr;
    }

    CGraphics graphScreen;
    HDC hdcScreen;
    hdcScreen = GetDC(nullptr);
    graphScreen.attach(hdcScreen);

    m_memGraph.create(nWidthGraph, getHeightBitmap(), &graphScreen, MEM_GRAPH_BITS);

    if (g_bAntialiasFontEnabled) {
        CGdiplusGraphicsLite *pmemGraph;

        pmemGraph = new CGdiplusGraphicsLite;
        pmemGraph->attach(m_memGraph.getHandle());
        m_pmemGraph = pmemGraph;
    } else {
        CGdiGraphicsLite *pmemGraph;

        pmemGraph = new CGdiGraphicsLite;
        pmemGraph->attach(m_memGraph.getHandle());
        m_pmemGraph = pmemGraph;
    }

    m_pmemGraph->setFont(&m_font);
    m_pmemGraph->setTextColor(CColor(RGB(255, 255, 255)));
    m_pmemGraph->setBgColor(CColor(RGB(0, 0, 0)));

    ReleaseDC(nullptr, hdcScreen);
}

void CRawGlyphBuilder::destroy() {
    if (m_pmemGraph) {
        delete m_pmemGraph;
        m_pmemGraph = nullptr;
    }

    m_memGraph.destroy();
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgb24 &bufGraph);

Glyph *CRawGlyphBuilder::buildGlyph(string &ch) {
    SIZE size;

    assert(m_pmemGraph);

    if (!m_pmemGraph->getTextExtentPoint32(ch.c_str(), ch.size(), &size)) {
        return nullptr;
    }

    if (size.cx > m_memGraph.width() + MARGIN_FONT * 2) {
        createMemGraph(size.cx + getHeightBitmap());
    }

    RawImageData *rawBuff = m_memGraph.getRawBuff();
    agg::rendering_buffer bufGraph(rawBuff->buff, rawBuff->width, rawBuff->height, rawBuff->stride);

    int nHeightBitmap = getHeightBitmap();

    uint8_t *pRowSrc = bufGraph.row_ptr(0);
    for (int y = 0; y < nHeightBitmap; y++) {
        memset(pRowSrc, 0, bufGraph.stride_abs());
        pRowSrc += bufGraph.stride();
    }

    if (!m_pmemGraph->textOut(MARGIN_FONT, MARGIN_FONT, ch.c_str(), ch.size())) {
        return nullptr;
    }

    Glyph *glyph = new Glyph;
    glyph->ch = ch;
    glyph->nWidth = (uint16_t)size.cx;

    agg::pixfmt_rgb24 pixf(bufGraph);
    copyBitmpDataFromGraphBuffer(glyph, pixf);

    return glyph;
}

};
