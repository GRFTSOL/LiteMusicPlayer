#pragma once

#ifndef MPlayerUI_LyricShowSingleRowObj_h
#define MPlayerUI_LyricShowSingleRowObj_h


#include "LyricShowObj.h"


class CLyricShowSingleRowObj : public CLyricShowObj {
public:
    CLyricShowSingleRowObj();
    virtual ~CLyricShowSingleRowObj();

public:
    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);
    int getLyricRightRowAlignPos(CRawGraph *canvas, LyricsLine *pLyricRow);
    int getLyricLeftRowAlignPos(CRawGraph *canvas, LyricsLine *pLyricRow);

    void drawCurLineFadeInNextLine(CRawGraph *canvas, LyricsLine *pLineCur, LyricsLine *pLineNext, int x, int y);

    void getCurLineFadeOutColor(LyricsLine *pLine, CColor clrIn, CColor clrOut, CColor &color);
    void getNextLineFadeInColor(LyricsLine *pLine, CColor clrIn, CColor clrOut, CColor &color);

public:
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName);
    cstr_t getClassName();
    static cstr_t               ms_szClassName;

protected:
    virtual int getAutoHeightLines() { return 1; }

};

#endif // !defined(MPlayerUI_LyricShowSingleRowObj_h)
