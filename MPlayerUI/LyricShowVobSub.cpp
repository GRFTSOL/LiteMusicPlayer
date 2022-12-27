// LyricShowVobSub.cpp: implementation of the CLyricShowVobSub class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "LyricShowVobSub.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
cstr_t CLyricShowVobSub::ms_szClassName = "LyricShowVobSub";

CLyricShowVobSub::CLyricShowVobSub()
{
    m_nDarkenTopArea = float(0.65);
    m_nDarkenBottomArea = float(0.65);

    m_nCurRowOld = -1;
    m_bClearedOld = false;
}

CLyricShowVobSub::~CLyricShowVobSub()
{

}

void CLyricShowVobSub::fastDraw(CRawGraph *canvas, CRect *prcUpdate)
{
    int            y;
    LyricsLine *pLyricRow = nullptr, *pLyricRowNext = nullptr;

    canvas->setFont(&m_font);

    if (prcUpdate)
        *prcUpdate = m_rcObj;

    if (m_pMLData == nullptr)
        return;

    int        nRowCur, nRowNext;
    int        nPlayPos;

    // get the current playing row
    nRowCur = m_pMLData->getCurPlayLine(m_lyrLines);
    if (nRowCur == -1)
    {
        // no lyrics, redraw background
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        return;
    }

    nPlayPos = m_pMLData->getPlayElapsedTime();

    pLyricRow = m_lyrLines[nRowCur];
    nRowNext = nRowCur +1;

    pLyricRowNext = nullptr;
    if (nRowNext < (int)m_lyrLines.size())
    {
        pLyricRowNext = m_lyrLines[nRowNext];
        if (nPlayPos < pLyricRowNext->nBegTime)
            pLyricRowNext = nullptr;
    }

    if (pLyricRowNext)
        y = getLineVertAlignPos() - getLineHeight() - m_nLineSpacing / 2;
    else
        y = getLineVertAlignPos() - getLineHeight() / 2;

    //
    // 增加此判断提高效率
    //
    if (nPlayPos >= pLyricRow->nBegTime && 
        nPlayPos <= pLyricRow->nEndTime &&
        nRowCur == m_nCurRowOld && prcUpdate != nullptr &&
        (m_LyricsDisplayOpt == DO_NORMAL && !isKaraoke()))
        return;

    // clear back buffer
    if (!m_bClearedOld || prcUpdate == nullptr)
    {
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        m_bClearedOld = true;
    }

    CRect    rcClip;

    rcClip.setLTRB(m_rcObj.left + m_nXMargin, 
                        m_rcObj.top + m_nYMargin,
                        m_rcObj.right - m_nXMargin,
                        m_rcObj.bottom - m_nYMargin);

    CRawGraph::CClipBoxAutoRecovery    autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    if (nPlayPos >= pLyricRow->nBegTime && 
        nPlayPos <= pLyricRow->nEndTime)
    {
        // 显示此行
        drawCurrentRow(canvas, pLyricRow, AUTO_CAL_X, y);
        y += getLineHeight();
        m_bClearedOld = false;
    }

    if (pLyricRowNext && nPlayPos >= pLyricRowNext->nBegTime && 
        nPlayPos <= pLyricRowNext->nEndTime)
    {
        // 显示此行
        drawCurrentRow(canvas, pLyricRowNext, AUTO_CAL_X, y);
        m_bClearedOld = false;
    }

    if (nPlayPos >= pLyricRow->nBegTime && 
        nPlayPos <= pLyricRow->nEndTime)
        m_nCurRowOld = nRowCur;
}

cstr_t CLyricShowVobSub::getClassName()
{
    return ms_szClassName;
}

bool CLyricShowVobSub::isKindOf(cstr_t szClassName)
{
    if (CLyricShowObj::isKindOf(szClassName))
        return true;

    return strcasecmp(szClassName, ms_szClassName) == 0;
}
