// LyricShowVobSub.h: interface for the CLyricShowVobSub class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICSHOWVOBSUB_H__144ECD5C_B59B_4CD8_8519_D8175CD64448__INCLUDED_)
#define AFX_LYRICSHOWVOBSUB_H__144ECD5C_B59B_4CD8_8519_D8175CD64448__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricShowObj.h"

class CLyricShowVobSub : public CLyricShowObj  
{
public:
    CLyricShowVobSub();
    virtual ~CLyricShowVobSub();

public:
    //    void UpdateDraw(uint32_t dwTimeElapsed);
    void fastDraw(CRawGraph *canvas, CRect *prcUpdate = nullptr);

    cstr_t getClassName();
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName);
    static cstr_t        ms_szClassName;

protected:
    virtual int getAutoHeightLines() { return 1; }

    // 使用下面的变量记录了上一次歌词的显示位置，在新的一次显示时，
    // 可以优化显示速度
    int                    m_nCurRowOld;        // 前一次显示的当前行
    bool                m_bClearedOld;        // 前一次显示是否清除过背景

};

#endif // !defined(AFX_LYRICSHOWVOBSUB_H__144ECD5C_B59B_4CD8_8519_D8175CD64448__INCLUDED_)
