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

    Glyph *buildGlyph(string &ch);

protected:
    int getHeightBitmap() { return m_font.getHeight() + MARGIN_FONT * 2; }

protected:
    cairo_t                     *m_cr;
    cairo_surface_t             *m_surface;

    CMLFont                     m_font;
    int                         m_nWidthGraph;

};

};

#endif // _RAW_GLYPH_BUILDER_H_
