#ifndef _RAW_GLYPH_BUILDER_H_
#define _RAW_GLYPH_BUILDER_H_

#pragma once

#include "GdiplusGraphicsLite.h"
#include "RawGraphData.h"


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
    void createMemGraph(int nWidthGraph);

protected:
    FontInfoEx                  m_fontInfo;
    CGdiplusGraphicsLite        m_fontGraph;
    CRawGraphData               m_fontMemGraph;

    CGdiplusFontPtr             m_fontLatin;
    CGdiplusFontPtr             m_fontOthers;

};

#endif // _RAW_GLYPH_BUILDER_H_
