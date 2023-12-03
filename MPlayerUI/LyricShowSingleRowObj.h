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

    void drawCurLineFadeInNextLine(CRawGraph *canvas, LyricsLine &lineCur, LyricsLine *lineNext, int x, int y);

public:
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName);
    cstr_t getClassName();
    static cstr_t               ms_szClassName;

protected:
    virtual int getAutoHeightLines() { return 1; }

};

#endif // !defined(MPlayerUI_LyricShowSingleRowObj_h)
