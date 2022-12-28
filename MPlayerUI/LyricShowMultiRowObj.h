#pragma once

/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LyricShowMultiRowObj.h
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#ifndef MPlayerUI_LyricShowMultiRowObj_h
#define MPlayerUI_LyricShowMultiRowObj_h


#include "LyricShowObj.h"


class CLyricShowMultiRowObj : public CLyricShowObj {
    UIOBJECT_CLASS_NAME_DECLARE(CLyricShowObj)
public:
    CLyricShowMultiRowObj();
    virtual ~CLyricShowMultiRowObj();

    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr) override;

    void invalidate() override;

protected:
    void fastestDraw_GetUpdateRectOfFadeInOut(CRawGraph *canvas, CRect *prcUpdate, int y, int nCurLine);
    void fastestDraw_GetUpdateRectOfNormal(CRawGraph *canvas, CRect *prcUpdate, int y);

    virtual int getAutoHeightLines() override {
        if (m_etDispSettings == ET_LYRICS_FLOATING_SETTINGS
            && (m_LyricsDisplayOpt == DO_FADEOUT_BG || m_LyricsDisplayOpt == DO_AUTO)) {
            return 3;
        } else {
            return 0;
        }
    }

protected:
    // 使用下面的变量记录了上一次歌词的显示位置，在新的一次显示时，
    // 可以优化显示速度
    int                         m_yDrawingOld;      // 上一次歌词显示的位置
    int                         m_nCurRowOld;       // 上一次显示的当前行歌词
    int                         m_nPrevLineAlphaOld, m_nextLineAlphaOld; // Old alpha value

};

#endif // !defined(MPlayerUI_LyricShowMultiRowObj_h)
