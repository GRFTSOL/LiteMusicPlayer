#ifndef _RAW_GLYPH_BUILDER_H_
#define _RAW_GLYPH_BUILDER_H_

#pragma once

namespace GfxRaw
{

class CRawGlyphBuilder {
public:
    CRawGlyphBuilder();
    virtual ~CRawGlyphBuilder();

    void init(const CFontInfo &font);

    int getHeight() const;

    void destroy();

    Glyph *buildGlyph(string &ch);

protected:
    void createMemGraph(int nWidthGraph);
    int getHeightBitmap() { return m_font.getHeight() + MARGIN_FONT * 2; }

protected:
    CFontInfo                   m_font;
    CGraphics                   *m_pmemGraph;
    CRawGraph                   m_memGraph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
