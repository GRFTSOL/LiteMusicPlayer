#ifndef _RAW_GLYPH_BUILDER_H_
#define _RAW_GLYPH_BUILDER_H_

#pragma once

#include "GfxFont.h"


class Glyph;

class CRawGlyphBuilder {
public:
    CRawGlyphBuilder();
    virtual ~CRawGlyphBuilder();

    void init(const FontInfoEx &font);

    int getHeight() const;

    void destroy();

    Glyph *buildGlyph(string &ch);

protected:
    int getHeightBitmap();

protected:
    FontInfoEx                  m_fontInfo;
    GfxFont                     m_fontLatin;
    GfxFont                     m_fontOthers;
    CRawGraphData               m_fontMemGraph;

};

#endif // _RAW_GLYPH_BUILDER_H_
