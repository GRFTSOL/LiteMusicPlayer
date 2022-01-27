#ifndef _RAW_GLYPH_BUILDER_H_
#define _RAW_GLYPH_BUILDER_H_

#pragma once

typedef const struct __CFDictionary * CFDictionaryRef;

#include "MLFont.h"

class CRawGlyphBuilder
{
public:
    CRawGlyphBuilder();
    virtual ~CRawGlyphBuilder();

    void init(const CFontInfo &font);

    int getHeight() const;

    void destroy();
    
    Glyph *buildGlyph(string &ch);

protected:
    int getHeightBitmap();

protected:
    CMLFont                     m_font;
    CRawGraphData               m_fontMemGraph;
    CFDictionaryRef             m_attributes;
    int                         m_nLineHeight;

};

#endif // _RAW_GLYPH_BUILDER_H_
