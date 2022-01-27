/********************************************************************
    Created  :    2002/01/04    21:40
    FileName :    LyricShowTwoRowObj.h
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#if !defined(AFX_LYRICSHOWTWOROWOBJ_H__7BC5AA20_7D13_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSHOWTWOROWOBJ_H__7BC5AA20_7D13_11D5_9E04_02608CAD9330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"

class CLyricShowTwoRowObj : public CLyricShowObj  
{
public:
    CLyricShowTwoRowObj();
    virtual ~CLyricShowTwoRowObj();

public:
    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);

    cstr_t getClassName();
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName);
    static cstr_t        ms_szClassName;

protected:
    virtual int getAutoHeightLines() { return 2; }

    // 使用下面的变量记录了上一次歌词的显示位置，在新的一次显示时，
    // 可以优化显示速度
    int                    m_nCurRowOld;        // 前一次显示的当前行
    int                    m_nRowTheOtherOld;    // 前一次显示的另一行
    CColor                m_clrTheOtherLineOld;

};

#endif // !defined(AFX_LYRICSHOWTWOROWOBJ_H__7BC5AA20_7D13_11D5_9E04_02608CAD9330__INCLUDED_)
