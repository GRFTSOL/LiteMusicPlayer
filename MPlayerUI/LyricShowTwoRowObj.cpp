/********************************************************************
    Created  :    2001年12月27日 0:01:36
    FileName :    LyricShowTwoRowObj.cpp
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "LyricShowTwoRowObj.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// set classname
cstr_t CLyricShowTwoRowObj::ms_szClassName = "LyricShowTwoRow";

CLyricShowTwoRowObj::CLyricShowTwoRowObj()
{
    m_nCurRowOld = -1;
    m_nRowTheOtherOld = -1;
    m_nDarkenTopArea = float(1.25);
    m_nDarkenBottomArea = float(1.25);
}

CLyricShowTwoRowObj::~CLyricShowTwoRowObj()
{
}

void CLyricShowTwoRowObj::fastDraw(CRawGraph *canvas, CRect *prcUpdate)
{
    if (m_pMLData == nullptr)
        return;

    int            y;
    LyricsLine *pLyricRow;
    int            nRowHeight;// center row height
    int            nRowTheOther = 0;
    int            nRowCur;
    LyricsLine *pLineTheOther = nullptr;

    canvas->setFont(&m_font);

    if (prcUpdate)
        *prcUpdate = m_rcObj;

    // get the current playing row
    nRowCur = m_pMLData->getCurPlayLine(m_lyrLines);
    if (nRowCur == -1)
    {
        // no lyrics, redraw background
        updateLyricDrawBufferBackground(canvas, m_rcObj);
        return;
    }

    pLyricRow = m_lyrLines[nRowCur];

    nRowHeight = getLineHeight();

    y = getLineVertAlignPos() - getLineHeight() + m_nLineSpacing / 2;
    if (y < m_rcObj.top + m_nYMargin)
        y = m_rcObj.top + m_rcObj.height() / 2 - getLineHeight() + m_nLineSpacing / 2;
    
    // 如果当前歌词行的显示时间超过一半，则预显示下一行歌词
    if ( (m_pMLData->getPlayElapsedTime() - pLyricRow->nBegTime) * 2 >= pLyricRow->nEndTime - m_pMLData->getPlayElapsedTime())
        nRowTheOther = nRowCur + 1;//bShowNextRow = true;
    else
    {
        nRowTheOther = nRowCur - 1;
        if (nRowTheOther < 0)
            // 当前行为最开始的一行，则显示下一行
            nRowTheOther = nRowCur + 1;
    }

    if (nRowTheOther >= (int)m_lyrLines.size())
    {
        // 当前行为最后一行，则显示前一行
        nRowTheOther = nRowCur - 1;
    }
    if (nRowTheOther < 0)
        nRowTheOther = -1;
    else
        pLineTheOther = m_lyrLines[nRowTheOther];

    int        yCurRow;
    int        yTheOtherRow;
    if (nRowCur % 2 == 0)
    {
        yCurRow = y;
        yTheOtherRow = y + nRowHeight;
    }
    else
    {
        yCurRow = y + nRowHeight;
        yTheOtherRow = y;
    }

    if (prcUpdate && m_nCurRowOld == nRowCur 
        && m_nRowTheOtherOld == nRowTheOther
        && m_LyricsDisplayOpt == DO_NORMAL)
    {
        if (!isKaraoke())
        {
            prcUpdate->setEmpty();
            // 如果没有使用KOROKE方式则直接返回
            return;
        }

        // 只重绘当前歌词行
        prcUpdate->top = yCurRow;
        prcUpdate->bottom = yCurRow + nRowHeight;
        if (prcUpdate->top < m_rcObj.top + m_nYMargin)
            prcUpdate->top = m_rcObj.top + m_nYMargin;
        if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
            prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;

        // 当前行歌词没有超出歌词显示区域
        prcUpdate->left = m_rcObj.left + m_nXMargin;
        prcUpdate->right = m_rcObj.right - m_nXMargin;

        // clear background
        updateLyricDrawBufferBackground(canvas, *prcUpdate);

        CRect    rcClip;

        rcClip = *prcUpdate;

        CRawGraph::CClipBoxAutoRecovery    autoCBR(canvas);
        canvas->setClipBoundBox(rcClip);

        // 显示当前歌词行
        drawCurrentRow(canvas, pLyricRow, AUTO_CAL_X, yCurRow);

        return;
    }
    else
    {
        m_nCurRowOld = nRowCur;
        m_nRowTheOtherOld = nRowTheOther;
    }

    // clear back buffer
    CRect    rcClip;
    if (prcUpdate)
    {
        prcUpdate->setLTRB(m_rcObj.left + m_nXMargin, y, m_rcObj.right - m_nXMargin, y + nRowHeight * 2);
        if (prcUpdate->top < m_rcObj.top + m_nYMargin)
            prcUpdate->top = m_rcObj.top + m_nYMargin;
        if (prcUpdate->bottom > m_rcObj.bottom - m_nYMargin)
            prcUpdate->bottom = m_rcObj.bottom - m_nYMargin;
        updateLyricDrawBufferBackground(canvas, *prcUpdate);
        rcClip = *prcUpdate;
    }
    else
    {
        updateLyricDrawBufferBackground(canvas, m_rcObj);


        rcClip.setLTRB(m_rcObj.left + m_nXMargin, 
                            m_rcObj.top + m_nYMargin,
                            m_rcObj.right - m_nXMargin,
                            m_rcObj.bottom - m_nYMargin);
    }
    CRawGraph::CClipBoxAutoRecovery    autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    // 显示当前歌词行
    drawCurrentRow(canvas, pLyricRow, AUTO_CAL_X, yCurRow);

    if (pLineTheOther)
    {
        // 显示前（后）一歌词行
        drawRow(canvas, pLineTheOther, AUTO_CAL_X, yTheOtherRow, 
            nRowTheOther < nRowCur ? LP_ABOVE_CUR_LINE : LP_BELOW_CUR_LINE);
    }
}

cstr_t CLyricShowTwoRowObj::getClassName()
{
    return ms_szClassName;
}

bool CLyricShowTwoRowObj::isKindOf(cstr_t szClassName)
{
    if (CLyricShowObj::isKindOf(szClassName))
        return true;

    return strcasecmp(szClassName, ms_szClassName) == 0;
}
