#include "MPlayerApp.h"
#include "MLCmd.h"
#include "LyricShowTextEditObj.h"
#include "LyricShowObj.h"
#include "../MediaTags/LrcParser.h"


#define TIMER_SPAN_LYR_PREVIEW_UPDATE   500

#define GLYPH_WIDTH_INVALID 0xFF

bool getTimeTagOfLine(cstr_t szLine, string &strTimeTag);

int getTimeTagValue(const char *szTag, size_t nLen);

using namespace _SkinEditCtrl;

CLyrEditSyntaxParser::CLyrEditSyntaxParser() {
    m_pLyrEdit = nullptr;
}

CLyrEditSyntaxParser::~CLyrEditSyntaxParser() {
}

void CLyrEditSyntaxParser::init(CSkinEditCtrl *pEdit) {
    m_pLyrEdit = (CLyricShowTextEditObj*)pEdit;

    CColor clrHilight(RGB(255, 255, 255)), clrLowlight(RGB(192, 192, 192));
    CColor clrTag(RGB(255, 0, 0)), clrEditLineBg(RGB(0, 0, 0));
    m_pLyrEdit->setColorTheme(clrHilight, clrLowlight, clrTag, clrEditLineBg);
}

void CLyrEditSyntaxParser::onNotifyParseLine(int nLine) {
    string strLine;
    CLyricShowTextEditObj::CLyrOneLine *pLine2;
    CLyricShowTextEditObj::CLyrOneLine *pLine = m_pLyrEdit->getLine(nLine);
    if (pLine && m_pLyrEdit->getTextOfLine(nLine, strLine)) {
        string strTime;
        int i;
        if (getTimeTagOfLine(strLine.c_str(), strTime)) {
            pLine->beginTime = getTimeTagValue(strTime.c_str(), strTime.size());
            pLine->bTimeLine = true;
            // DBG_LOG2("Line: %d Changed: %s", nLine, strLine.c_str());
            for (i = 0; i < (int)strTime.size(); i++) {
                (*pLine)[i].clrIndex = C_TAG;
            }
            for (i = (int)strTime.size(); i < (int)pLine->size(); i++) {
                (*pLine)[i].clrIndex = C_LOWLIGHT;
            }

            if (nLine == m_pLyrEdit->getLineCount() - 1 || pLine->endTime < pLine->beginTime) {
                pLine->endTime = pLine->beginTime + 1000 * 5;
            }

            do {
                nLine--;
                pLine2 = m_pLyrEdit->getLine(nLine);
            }
            while (pLine2 && !pLine2->bTimeLine);

            if (pLine2 && pLine2->bTimeLine) {
                pLine2->endTime = pLine->beginTime;
            }
        } else {
            // other text ...
            for (int i = 0; i < (int)pLine->size(); i++) {
                (*pLine)[i].clrIndex = C_LOWLIGHT;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// CLyricShowTextEditObj
//
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowTextEditObj, "LyricShowTextEdit");

CLyricShowTextEditObj::CLyricShowTextEditObj() {
    m_bEnableBorder = false;

    m_findReplaceStatus = FRS_NONE;

    m_lyrEditSyntaxParser.init(this);
    setEditSyntaxParser(&m_lyrEditSyntaxParser);

    m_msgNeed |= UO_MSG_WANT_COMMAND;
    m_nEditorStyles |= S_MULTILINE;

    m_bDiscardLyrChangeEvent = false;
    m_nTimerIDUpdateLyrics = 0;
}

CLyricShowTextEditObj::~CLyricShowTextEditObj() {
}

void CLyricShowTextEditObj::onCreate() {
    setEditNotification(this);

    CSkinEditCtrl::onCreate();

    GET_ID_BY_NAME(CID_MATCHCASE);
    GET_ID_BY_NAME(CID_FIND_PREV);
    GET_ID_BY_NAME(CID_FIND_NEXT);
    GET_ID_BY_NAME(CID_E_FIND);
    GET_ID_BY_NAME(CID_REPLACE);
    GET_ID_BY_NAME(CID_REPLACE_ALL);
    GET_ID_BY_NAME(CID_E_REPLACE);
    GET_ID_BY_NAME(CID_E_ARTIST);
    GET_ID_BY_NAME(CID_E_TITLE);
    GET_ID_BY_NAME(CID_E_ALBUM);
    GET_ID_BY_NAME(CID_E_BY);
    GET_ID_BY_NAME(CID_E_OFFSET);
    GET_ID_BY_NAME(CID_E_MEDIA_LENGTH);
    GET_ID_BY_NAME4(CID_RERESH_ARTIST, CID_RERESH_ALBUM, CID_RERESH_TITLE, CID_RERESH_BY);
    GET_ID_BY_NAME2(CID_RERESH_MEDIA_LENGTH, CID_TB_SEEK_BAR);

    CUIObject *pTagEditor = m_pSkin->getUIObjectById(getIDByName("CID_C_LYR_TAG"));
    if (pTagEditor) {
        bool bVisible = g_profile.getBool("LyrTagEdit", true);
        if (!bVisible) {
            pTagEditor->setVisible(false, false);
        }
    }

    onLyricsChanged();

    registerHandler(MPlayerApp::getEventsDispatcher(), ET_LYRICS_ON_SAVE_EDIT, ET_LYRICS_EDITOR_RELOAD_TAG, ET_LYRICS_CHANGED, ET_LYRICS_DRAW_UPDATE);
    registerHandler(MPlayerApp::getEventsDispatcher(), ET_LYRICS_DISPLAY_SETTINGS, ET_PLAYER_CUR_MEDIA_CHANGED);

    updateTextFontColor();

    m_pSkin->setUIObjectText(CID_E_FIND, g_profile.getString("FindWhat", ""));
    m_pSkin->setUIObjectText(CID_E_REPLACE, g_profile.getString("ReplaceWith", ""));

    // If there's no lyrics opened, fill media info.
    if (!g_currentLyrics.hasLyricsOpened()) {
        onCommand(ID_AUTO_FILL_LYR_INFO);
    }

    bool isEditorToolbarVisible = g_profile.getBool("LyrEditorTB_Visible", false);
    m_pSkin->setUIObjectVisible(ID_TB_LYR_EDIT, isEditorToolbarVisible, false);
    m_pSkin->setUIObjectVisible(CID_TB_SEEK_BAR, !isEditorToolbarVisible, false);
}

void CLyricShowTextEditObj::onDestroy() {
    onSaveLyrics();

    string strFindWhat = m_pSkin->getUIObjectText(CID_E_FIND);
    string strReplaceWith = m_pSkin->getUIObjectText(CID_E_REPLACE);
    g_profile.writeString("FindWhat", strFindWhat.c_str());
    g_profile.writeString("ReplaceWith", strReplaceWith.c_str());

    CSkinEditCtrl::onDestroy();
}

void CLyricShowTextEditObj::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_LYRICS_ON_SAVE_EDIT) {
        onSaveLyrics();
    } else if (pEvent->eventType == ET_LYRICS_EDITOR_RELOAD_TAG) {
        updateLyricsProperties(true);
    } else if (pEvent->eventType == ET_LYRICS_CHANGED) {
        onLyricsChanged();
    } else if (pEvent->eventType == ET_LYRICS_DRAW_UPDATE) {
        onPlayTimeChangedUpdate();
    } else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED) {
        if (!g_currentLyrics.hasLyricsOpened()) {
            onCommand(ID_AUTO_FILL_LYR_INFO);
        }
    } else if (pEvent->eventType == ET_LYRICS_DISPLAY_SETTINGS) {
        cstr_t szProperty = pEvent->name.c_str();

        if (isPropertyName(szProperty, "Ed_HighColor")
            || isPropertyName(szProperty, "Ed_LowColor")
            || isPropertyName(szProperty, "Ed_BgColor")
            || isPropertyName(szProperty, "Ed_TagColor")
            || isPropertyName(szProperty, "Ed_FocusLineBgColor")
            || isPropertyName(szProperty, "Font")) {
            updateTextFontColor();
            invalidate();
        }
    }
}

void CLyricShowTextEditObj::autoVScrollDown(int nDownLine) {
    if (m_nCaretRow + nDownLine < m_pVertScrollBar->getPage() / 2) {
        return;
    }

    if (!m_pVertScrollBar->isEnabled()) {
        return;
    }

    if (m_pVertScrollBar->getScrollPos() >= m_pVertScrollBar->getMax()) {
        return;
    }

    m_nTopVisibleLine += nDownLine;

    if (m_nTopVisibleLine >= (int)m_vLines.size()) {
        m_nTopVisibleLine = (int)m_vLines.size() - 1;
    }
    if (m_nTopVisibleLine < 0) {
        m_nTopVisibleLine = 0;
    }

    m_pVertScrollBar->setScrollPos(m_nTopVisibleLine);
    showCaret();

    invalidate();
}

bool CLyricShowTextEditObj::onKeyDown(uint32_t nChar, uint32_t nFlags) {
#ifdef _MAC_OS
    bool ctrl = isModifierKeyPressed(MK_COMMAND, nFlags);
#else
    bool ctrl = isModifierKeyPressed(MK_CONTROL, nFlags);
#endif
    bool shift = isModifierKeyPressed(MK_SHIFT, nFlags);

    if (!ctrl && !shift) {
        if (nChar == VK_F3) {
            findNext();
            return true;
        } else if ((nChar == VK_UP || nChar == VK_DOWN) && !isSelected()) {
            //
            // Adjust the time tag value, with up/down key.
            //
            string strLine, strTimeTag;

            if (getTextOfLine(m_nCaretRow, strLine)
                && getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                //                 int        nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol;
                //                 bool    bSelectedEmpty;
                //                 sortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
                //                 if (!bSelectedEmpty && nBegSelRow == nEndSelRow
                //                     && nBegSelRow == m_nCaretRow)
                // [00:12.00]
                //   ^  ^  ^
                const int nTimeTagStdLen = 10;
                const int nMinutePos = 2;
                const int nSecondPos = 5;
                const int nMSPos = 8;
                if (strTimeTag.size() == nTimeTagStdLen
                    && (m_nCaretCol == nMinutePos || m_nCaretCol == nSecondPos || m_nCaretCol == nMSPos)) {
                    // get value
                    int n = atoi(strTimeTag.c_str() + m_nCaretCol - 1);
                    int nUnit = 1, nMax = 60;

                    if (m_nCaretCol == nMSPos) {
                        nUnit = 10;
                        nMax = 100;
                    } else if (m_nCaretCol == nMinutePos) {
                        nMax = 99;
                    }

                    // set new value
                    if (nChar == VK_DOWN) {
                        n += nUnit;
                        if (n >= nMax) {
                            n = n % nUnit;
                        }
                    } else {
                        n -= nUnit;
                        if (n < 0) {
                            if (m_nCaretCol == nMinutePos) {
                                n = 0;
                            } else {
                                n += nMax;
                            }
                        }
                    }

                    // Replace selected tag value.
                    int nCaretColOld = m_nCaretCol;
                    setSel(m_nCaretRow, m_nCaretCol - 1, m_nCaretRow, m_nCaretCol + 1);
                    replaceSel(stringPrintf("%02d", n).c_str());
                    setCaret(m_nCaretRow, nCaretColOld);

                    return true;
                }
            }
        }
    }

    if (ctrl) {
        switch (nChar) {
            case VK_F:
                if (isModifierKeyPressed(MK_ALT, nFlags)) {
                    showFindDialog(false);
                } else {
                    showFindDialog();
                }
                return true;
            case VK_H:
                showFindDialog(false);
                return true;
        }
    }

    return CSkinEditCtrl::onKeyDown(nChar, nFlags);
}

bool CLyricShowTextEditObj::findText(cstr_t szText, uint32_t dwFlags) {
    bool bDown = isFlagSet(dwFlags, FTF_DOWN);
    bool bCase = isFlagSet(dwFlags, FTF_MATCH_CASE);
    // bool        bWholeWord = isFlagSet(dwFlags, FTF_WHOLEWORD);

    int nLine, nCol;
    int nBegSelRow, nEndSelRow;
    int nBegSelCol, nEndSelCol;
    bool bSelectedEmpty = true;

    getCaret(nLine, nCol);

    if (m_nBegSelRow != -1) {
        bSelectedEmpty = false;
        sortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
        if (!bSelectedEmpty) {
            if (bDown) {
                nLine = nEndSelRow;
                nCol = nEndSelCol;
            } else {
                nLine = nBegSelRow;
                nCol = nBegSelCol;
            }
        }
    }

    if (bDown) {
        // search down
        size_t nLineCount = m_vLines.size();
        int k;

        for (int i = nLine; i < nLineCount; i++) {
            COneLine *pLine = m_vLines[i];

            for (; nCol < (int)pLine->size(); nCol++) {
                cstr_t szTextCmp = szText;
                for (k = nCol; k < (int)pLine->size(); k++) {
                    CGlyph &glyph = (*pLine)[k];
                    if (bCase) {
                        if (glyph.cmp(szTextCmp)) {
                            szTextCmp += glyph.length();
                        } else {
                            break;
                        }
                    } else {
                        if (glyph.iCmp(szTextCmp)) {
                            szTextCmp += glyph.length();
                        } else {
                            break;
                        }
                    }
                }
                if (*szTextCmp == '\0') {
                    // Found it.
                    m_nBegSelRow = i;
                    m_nBegSelCol = nCol;
                    m_nEndSelRow = i;
                    m_nEndSelCol = k;

                    onNotifySelChanged();

                    setCaret(i, k);
                    return true;
                }
            }
            nCol = 0;
        }
    } else {
        // search up
        int k;
        size_t nLenCmp = strlen(szText);

        // Previous character...
        prevPos(nLine, nCol);

        if (nCol >= (int)m_vLines[nLine]->size()) {
            nCol = (int)m_vLines[nLine]->size() - 1;
        }
        for (int i = nLine; i >= 0; i--) {
            COneLine *pLine = m_vLines[i];

            for (; nCol >= 0; nCol--) {
                cstr_t szTextCmp = szText + nLenCmp - 1;
                for (k = nCol; k >= 0 && szTextCmp >= szText; k--) {
                    CGlyph &glyph = (*pLine)[k];
                    if (bCase) {
                        if (glyph.reverseCmp(szTextCmp)) {
                            szTextCmp -= glyph.length();
                        } else {
                            break;
                        }
                    } else {
                        if (glyph.reverseiCmp(szTextCmp)) {
                            szTextCmp -= glyph.length();
                        } else {
                            break;
                        }
                    }
                }
                if (szTextCmp == szText - 1) {
                    // Found it.
                    m_nBegSelRow = i;
                    m_nBegSelCol = nCol + 1;
                    m_nEndSelRow = i;
                    m_nEndSelCol = k + 1;

                    onNotifySelChanged();

                    setCaret(i, nCol + 1);
                    return true;
                }
            }
            if (i > 0) {
                nCol = (int)m_vLines[i - 1]->size() - 1;
            }
        }
    }

    return false;
}

CLyricShowTextEditObj::CLyrOneLine *CLyricShowTextEditObj::getLine(int nLine) {
    if (nLine >= 0 && nLine < (int)m_vLines.size()) {
        return (CLyrOneLine*)m_vLines[nLine];
    } else {
        return nullptr;
    }
}

void CLyricShowTextEditObj::draw(CRawGraph *canvas) {
    bool bSelectedEmpty = true;

    if (m_nBegSelRow != -1) {
        int nBegSelRow, nEndSelRow, nBegSelCol, nEndSelCol;
        bSelectedEmpty = false;
        sortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
    }

    if (bSelectedEmpty) {
        m_nTopVisibleLineOld = -1;

        reDraw(canvas);

        auto oldBgType = m_bgType;
        m_bgType = BG_NONE;
        CSkinScrollFrameCtrlBase::draw(canvas);
        m_bgType = oldBgType;
        return;
    }

    CSkinEditCtrl::draw(canvas);
}

/**
 * getFastDrawRect 只有 onPlayTimeChangedUpdate 会调用
 */
CRect CLyricShowTextEditObj::getFastDrawRect() {
    CRect rc = m_rcContent;
    CRect rcUpdate = rc;

    int xStart = rc.left + m_xMargin;
    int nTimePos = g_currentLyrics.getPlayElapsedTime();

    if (m_nTopVisibleLine == m_nTopVisibleLineOld) {
        // Fast draw ...?
        if (m_nCurPosLineOld >= 0 && m_nCurPosLineOld < (int)m_vLines.size()) {
            CLyrOneLine *pLyrLine = (CLyrOneLine*)m_vLines[m_nCurPosLineOld];
            if (pLyrLine->bTimeLine) {
                if (pLyrLine->beginTime <= nTimePos && nTimePos <= pLyrLine->endTime) {
                    int nTimeTagLen;
                    // current line of cur pos doesn't change, only update this line
                    for (nTimeTagLen = 0; nTimeTagLen < (int)pLyrLine->size(); nTimeTagLen++) {
                        if ((*pLyrLine)[nTimeTagLen].isEqual(']')) {
                            nTimeTagLen++;
                            break;
                        }
                    }

                    int nXOfCurPosLine = 0;
                    int dt = (pLyrLine->endTime - pLyrLine->beginTime);
                    assert(dt > 0);
                    if (dt > 0) {
                        nXOfCurPosLine = int(pLyrLine->size() - nTimeTagLen) * (nTimePos - pLyrLine->beginTime) / dt + nTimeTagLen;
                    }
                    if (nXOfCurPosLine > (int)pLyrLine->size()) {
                        nXOfCurPosLine = (int)pLyrLine->size();
                    }

                    if (m_nXOfCurPosLineOld == nXOfCurPosLine) {
                        // no need to update
                        rcUpdate.setEmpty();
                    } else {
                        // only update the current line.
                        rcUpdate.top = rc.top + m_yMargin + getLineDy() * (m_nCurPosLineOld - m_nTopVisibleLine);
                        rcUpdate.bottom = rcUpdate.top + getLineDy();
                    }
                    return rcUpdate;
                }
            }
        }

        if (m_nCurPosLineOld == -1) {
            // current line isn't being displayed.
            int y = m_yMargin;
            int yMax = rc.bottom - m_yMargin;

            for (int i = 0; i < (int)m_vLines.size(); i++) {
                if (y + getLineDy() > yMax && i > m_nTopVisibleLine) {
                    break;
                }
                CLyrOneLine *pLyrLine = (CLyrOneLine *)m_vLines[i];
                if (pLyrLine->bTimeLine &&
                    pLyrLine->beginTime <= nTimePos && nTimePos <= pLyrLine->endTime) {
                    m_nCurPosLineOld = i;
                    break;
                }
                y += getLineDy();
            }
            if (m_nCurPosLineOld == -1) {
                rcUpdate.setEmpty();
                return rcUpdate;
            }
        }
    }

    return rcUpdate;
}

void CLyricShowTextEditObj::reDraw(CRawGraph *canvas) {
    CRect &rc = m_rcContent;

    int y, xStart, xMax, yMax;
    int i;
    CLyrOneLine *pLyrLine;
    CRect rcClip;

    canvas->setFont(m_font.getFont());

    fillGraph(canvas, rc, CN_BG);

    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    rcClip.setLTRB(rc.left + m_xMargin, rc.top + m_yMargin, rc.right - m_xMargin, rc.bottom - m_yMargin);
    canvas->setClipBoundBox(rcClip);

    xStart = rcClip.left;
    y = rcClip.top;
    xMax = rcClip.right;
    yMax = rcClip.bottom;
    assert(m_nTopVisibleLine >= 0 && m_nTopVisibleLine < (int)m_vLines.size());

    int nTimePos = g_currentLyrics.getPlayElapsedTime();

    m_nCurPosLineOld = -1;
    m_nXOfCurPosLineOld = -1;
    m_nTopVisibleLineOld = m_nTopVisibleLine;

    for (i = m_nTopVisibleLine; i < (int)m_vLines.size(); i++) {
        if (y + getLineDy() > yMax && i > m_nTopVisibleLine) {
            break;
        }

        if (m_nCaretRow == i) {
            fillGraph(canvas, m_xMargin, y, xMax - m_xMargin * 2, getLineDy(), CN_EDIT_LINE_BG);
        }

        pLyrLine = (CLyrOneLine *)m_vLines[i];
        if (!pLyrLine->bTimeLine) {
            drawLine(canvas, pLyrLine, xStart, xMax, y);
        } else if (pLyrLine->endTime < nTimePos) {
            // draw in highlight
            CColor clrOld;
            clrOld = getColor(CLyrEditSyntaxParser::C_LOWLIGHT);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, getColor(CLyrEditSyntaxParser::C_HILIGHT));
            CSkinEditCtrl::drawLine(canvas, pLyrLine, xStart, xMax, y);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
        } else if (pLyrLine->beginTime >= nTimePos) {
            // draw in lowlight
            CSkinEditCtrl::drawLine(canvas, pLyrLine, xStart, xMax, y);
        } else if (pLyrLine->beginTime >= pLyrLine->endTime) {
            // draw in highlight
            CColor clrOld;
            clrOld = getColor(CLyrEditSyntaxParser::C_LOWLIGHT);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, getColor(CLyrEditSyntaxParser::C_HILIGHT));
            CSkinEditCtrl::drawLine(canvas, pLyrLine, xStart, xMax, y);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
        } else if (m_nCurPosLineOld == -1) {
            m_nCurPosLineOld = i;
            drawLineGradual(canvas, pLyrLine, xStart, xMax, y);
        } else {
            CColor clrOld;
            clrOld = getColor(CLyrEditSyntaxParser::C_LOWLIGHT);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, getColor(CLyrEditSyntaxParser::C_HILIGHT));
            drawLine(canvas, pLyrLine, xStart, xMax, y);
            setColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrOld);
        }

        y += getLineDy();
    }

    m_caret.draw(canvas);
}

void CLyricShowTextEditObj::drawLineGradual(CRawGraph *canvas, COneLine *pLine, int x, int xMax, int y) {
    CLyrOneLine *pLyrLine = (CLyrOneLine *)pLine;

    if (!pLyrLine->bTimeLine) {
        CSkinEditCtrl::drawLine(canvas, pLine, x, xMax, y);
        return;
    }

    int nTimePos = g_currentLyrics.getPlayElapsedTime();

    uint8_t byClrLast = 0;
    int k;
    int nTimeTagLen;

    x -= m_nScrollPosx;

    canvas->setTextColor(m_vClrTable[byClrLast]);

    if (nTimePos < pLyrLine->beginTime) {
        nTimePos = pLyrLine->beginTime;
    } else if (nTimePos > pLyrLine->endTime) {
        nTimePos = pLyrLine->endTime;
    }

    // draw time tag
    for (k = 0; k < (int)pLine->size(); k++) {
        CGlyph &glyph = (*pLine)[k];
        assert(glyph.width != GLYPH_WIDTH_INVALID);

        if (glyph.clrIndex != byClrLast) {
            if (glyph.clrIndex >= m_vClrTable.size()) {
                byClrLast = 0;
            } else {
                byClrLast = glyph.clrIndex;
            }
            canvas->setTextColor(m_vClrTable[byClrLast]);
        }
        if (x + glyph.width < m_xMargin) {
            x += glyph.width;
            continue;
        }
        if (x + glyph.width > xMax) {
            return;
        }
        if (!glyph.isEqual('\t')) {
            canvas->textOut(x, y, glyph.chGlyph, (int)glyph.chGlyph.size());
        }
        x += glyph.width;
        if (glyph.isEqual(']')) {
            break;
        }
    }
    k++;
    nTimeTagLen = k;

    int nXOfCurPosLine = 0;
    int dt = (pLyrLine->endTime - pLyrLine->beginTime);
    assert(dt > 0);
    if (dt > 0) {
        nXOfCurPosLine = int(pLyrLine->size() - nTimeTagLen) * (nTimePos - pLyrLine->beginTime) / dt + nTimeTagLen;
    }

    if (nXOfCurPosLine > (int)pLyrLine->size()) {
        nXOfCurPosLine = (int)pLyrLine->size();
    }

    m_nXOfCurPosLineOld = nXOfCurPosLine;

    canvas->setTextColor(getColor(CLyrEditSyntaxParser::C_HILIGHT));
    for (; k < nXOfCurPosLine; k++) {
        CGlyph &glyph = (*pLine)[k];
        assert(glyph.width != GLYPH_WIDTH_INVALID);

        if (x + glyph.width < m_xMargin) {
            x += glyph.width;
            continue;
        }
        if (x + glyph.width > xMax) {
            return;
        }
        if (!glyph.isEqual('\t')) {
            canvas->textOut(x, y, glyph.chGlyph, (int)glyph.chGlyph.size());
        }
        x += glyph.width;
    }

    canvas->setTextColor(getColor(CLyrEditSyntaxParser::C_LOWLIGHT));
    for (; k < (int)pLine->size(); k++) {
        CGlyph &glyph = (*pLine)[k];
        assert(glyph.width != GLYPH_WIDTH_INVALID);

        if (x + glyph.width < m_xMargin) {
            x += glyph.width;
            continue;
        }
        if (x + glyph.width > xMax) {
            return;
        }
        if (!glyph.isEqual('\t')) {
            canvas->textOut(x, y, glyph.chGlyph, (int)glyph.chGlyph.size());
        }
        x += glyph.width;
    }
}

void CLyricShowTextEditObj::setColorTheme(CColor &clrHilight, CColor &clrLowlight, CColor &clrTag, CColor &clrEditLineBg) {
    setColor(CLyrEditSyntaxParser::C_HILIGHT, clrHilight);
    setColor(CLyrEditSyntaxParser::C_LOWLIGHT, clrLowlight);
    setColor(CLyrEditSyntaxParser::C_TAG, clrTag);

    setSelColor(clrHilight, clrEditLineBg);
    setColor(CN_EDIT_LINE_BG, clrEditLineBg);
}

bool CLyricShowTextEditObj::isTextChanged() {
    return m_undoMgr.canUndo();
}

void CLyricShowTextEditObj::onCmdDelete() {
    if (isSelected() || m_nCaretCol != 0) {
        CSkinEditCtrl::onCmdDelete();
        return;
    }

    //
    // delete the whole tag, if the first char of tag is deleted.
    //

    string strToDel;
    int nContentBegPos;
    string strLine, strContent;

    if (!getTextOfLine(m_nCaretRow, strLine)) {
        return;
    }

    if (getTimeTagOfLine(strLine.c_str(), strToDel)) {
        nContentBegPos = (int)strToDel.size();
    } else {
        CSkinEditCtrl::onCmdDelete();
        return;
    }

    setSel(m_nCaretRow, m_nCaretCol, m_nCaretRow, nContentBegPos);

    CSkinEditCtrl::onCmdDelete();
}

int CLyricShowTextEditObj::getCaretLineHomePos() {
    string strLine, strTimeTag;
    int nColHome = 0;

    if (getTextOfLine(m_nCaretRow, strLine)) {

        if (getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
            nColHome = (int)strTimeTag.size();
        }

        if (nColHome == m_nCaretCol) {
            nColHome = 0;
        }
    }

    return nColHome;
}

bool getTimeTagOfLine(cstr_t szLine, string &strTimeTag) {
    cstr_t szPtr = szLine;
    int i;

    strTimeTag = "";

    // ignore blank space
    while (*szPtr == ' ' || *szPtr == '\t') {
        szPtr++;
    }
    if (*szPtr != '[') {
        return false;
    }
    szPtr++;

    // ignore minute
    if (*szPtr == '-') {
        szPtr++;
    }
    for (i = 0; i < 3 && isDigit(*szPtr); i++) {
        szPtr++;
    }

    // :
    if (*szPtr != ':') {
        return false;
    }
    szPtr++;

    // ignore sec
    if (*szPtr == '-') {
        szPtr++;
    }
    for (i = 0; i < 3 && isDigit(*szPtr); i++) {
        szPtr++;
    }

    if (*szPtr == ']') {
        szPtr++;
        strTimeTag.append(szLine, (int)(szPtr - szLine));
        return true;
    }
    if (*szPtr == '.' || *szPtr == ':') {
        szPtr++;

        // ignore ms
        if (*szPtr == '-') {
            szPtr++;
        }
        for (i = 0; i < 3 && isDigit(*szPtr); i++) {
            szPtr++;
        }

        if (*szPtr == ']') {
            szPtr++;
            strTimeTag.append(szLine, (int)(szPtr - szLine));
            return true;
        }
    }

    return false;
}

int getTimeTagValue(const char *szTag, size_t nLen) {
    int nTime;

    nTime = 0;

    if (*szTag != '[') {
        goto S_ERROR;
    }

    szTag++;
    // time tag ?
    // seek to ':'
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    nTime += atoi(szTag) * 60 * 1000; // nTime += atoi(szTag) * 60s * 1000ms
    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag ++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    // ':'
    if (*szTag != ':' && *szTag != '.') {
        goto S_ERROR;
    }
    szTag ++;

    // number
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    nTime += atoi(szTag) * 1000; // nTime += atoi(szTag) * 1000ms
    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag ++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    if (*szTag == '.' || *szTag == ':') {
        szTag ++;
        // new version of lrc file
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }

        if (isDigit(szTag[1])) {
            // .000 or // .00
            if (isDigit(szTag[2])) {
                nTime += atoi(szTag);
            } else {
                nTime += atoi(szTag) * 10;
            }
        } else {
            nTime += atoi(szTag) * 100;    // .0
        }
        if (*szTag == '-') {
            szTag++;
        }

        while (isDigit(*szTag)) {
            szTag ++;
        }
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }
    }
    if (*szTag != ']') {
        goto S_ERROR;
    }

S_ERROR:
    return nTime;
}

//////////////////////////////////////////////////////////////////////

bool CLyricShowTextEditObj::onCommand(uint32_t nId) {
    CSkinWnd::CAutoRedrawLock arl(m_pSkin);

    if (nId == CID_FIND_PREV) {
        findNext(false);
    } else if (nId == CID_FIND_NEXT) {
        findNext();
    } else if (nId == CID_REPLACE) {
        // replace
        bool matchCase = m_pSkin->isButtonChecked(CID_MATCHCASE);
        string strFindWhat = m_pSkin->getUIObjectText(CID_E_FIND);
        string strReplaceWith = m_pSkin->getUIObjectText(CID_E_REPLACE);
        string strSel;

        if (strFindWhat.empty()) {
            return true;
        }

        selectionToText(strSel);

        auto cmpFunc = matchCase ? strcmp : strcasecmp;
        if (strSel.size() && cmpFunc(strSel.c_str(), strFindWhat.c_str()) == 0) {
            replaceSel(strReplaceWith.c_str());
        }
        findNext();
    } else if (nId == CID_REPLACE_ALL) {
        // replace all
        string strFindWhat = m_pSkin->getUIObjectText(CID_E_FIND);
        string strReplaceWith = m_pSkin->getUIObjectText(CID_E_REPLACE);

        if (strFindWhat.empty()) {
            return true;
        }

        setCaret(0, 0);
        m_nBegSelRow = -1;
        m_nBegSelCol = -1;

        int countReplaced = 0;
        AutoBatchUndo autoBatchUndo(this);
        while (findNext(true, false)) {
            replaceSel(strReplaceWith.c_str());
            countReplaced++;
        }
    } else if (nId == CID_RERESH_ARTIST) {
        m_pSkin->setUIObjectText(CID_E_ARTIST, g_player.getArtist());
    } else if (nId == CID_RERESH_TITLE) {
        m_pSkin->setUIObjectText(CID_E_TITLE, g_player.getTitle());
    } else if (nId == CID_RERESH_ALBUM) {
        m_pSkin->setUIObjectText(CID_E_ALBUM, g_player.getAlbum());
    } else if (nId == CID_RERESH_BY) {
        string strLogName = g_profile.getString("LoginName", "");
        if (strLogName.size()) {
            m_pSkin->setUIObjectText(CID_E_BY, strLogName.c_str());
        }
    } else if (nId == CID_RERESH_MEDIA_LENGTH) {
        LyricsProperties &prop = g_currentLyrics.properties();
        prop.setMediaLength(g_player.getMediaLength() / 1000);
        m_pSkin->setUIObjectText(CID_E_MEDIA_LENGTH, prop.mediaLength.c_str());
    }

    switch (nId) {
    case ID_JUMP_TO_CUR_LINE:
        {
            int nLine, nCol, nTime;
            string strLine, strTimeTag;

            getCaret(nLine, nCol);
            getTextOfLine(nLine, strLine);
            if (getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                nTime = getTimeTagValue(strTimeTag.c_str(), strTimeTag.size())
                - g_currentLyrics.getOffsetTime();
                g_player.seekTo(nTime);
            }
            return true;
        }
        break;
    case ID_DEL_TAG:
        {
            string strLine, strTimeTag;

            int nBegSelRow, nEndSelRow;

            if (m_nBegSelRow != -1) {
                if (m_nBegSelRow <= m_nEndSelRow) {
                    nBegSelRow = m_nBegSelRow;
                    nEndSelRow = m_nEndSelRow;
                } else {
                    nBegSelRow = m_nEndSelRow;
                    nEndSelRow = m_nBegSelRow;
                }
            } else {
                int nLine, nCol;

                getCaret(nLine, nCol);
                nBegSelRow = nEndSelRow = nLine;
            }

            AutoBatchUndo autoBatchUndo(this);

            for (int i = nBegSelRow; i <= nEndSelRow; i++) {
                getTextOfLine(i, strLine);

                if (!getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    continue;
                }

                setSelOfLine(i, 0, (int)strTimeTag.size());
                removeSel();
            }

            return true;
        }
        break;
    case ID_TOGGLE_LYR_EDIT_TOOLBAR:
        {
            CUIObject *pToolbar = m_pSkin->getUIObjectById(ID_TB_LYR_EDIT);
            if (pToolbar) {
                pToolbar->setVisible(!pToolbar->isVisible(), true);
                g_profile.writeInt("LyrEditorTB_Visible", pToolbar->isVisible());
                m_pSkin->setUIObjectVisible(CID_TB_SEEK_BAR, !pToolbar->isVisible(), false);
            }
            break;
        }
    case ID_INSERTTAG:
        syncTimeTag(false);
        break;
    case ID_INSERTTAG_DOWN:
        syncTimeTag(true);
        break;
    case ID_FORWARD_CUR_LINE:
        adjustSyncTimeOfSelected(true);
        break;
    case ID_BACKWARD_CUR_LINE:
        adjustSyncTimeOfSelected(false);
        break;
    case ID_FORWARD_REMAIN_LINES:
    case ID_BACKWARD_REMAIN_LINES:
        {
            int nLine, nCol;
            string strLine, strTimeTag;
            int nTime;

            getCaret(nLine, nCol);

            AutoBatchUndo autoBatchUndo(this);

            for (int i = nLine; getTextOfLine(i, strLine); i++) {
                if (getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    nTime = getTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
                    if (nId == ID_FORWARD_REMAIN_LINES) {
                        nTime += 200;
                    } else {
                        nTime -= 200;
                    }

                    string newTimeTag = formatLrcTimeTag(nTime, true);

                    setSelOfLine(i, 0, (int)strTimeTag.size());
                    replaceSel(newTimeTag.c_str());
                }
            }

            setCaret(nLine, nCol);
            return true;
        }
    case ID_EDIT_UNDO:
        onCmdUndo();
        break;
    case ID_EDIT_REDO:
        onCmdRedo();
        break;
    case ID_EDIT_CUT:
        onCmdCut();
        break;
    case ID_EDIT_COPY:
        onCmdCopy();
        break;
    case ID_EDIT_PASTE:
        onCmdPaste();
        break;
    case ID_EDIT_DELETE:
        onCmdDelete();
        break;
    case ID_EDIT_FIND:
        showFindDialog();
        break;
    case ID_EDIT_FINDNEXT:
        findNext();
        break;
    case ID_EDIT_REPLACE:
        showFindDialog(false);
        break;
    case ID_AUTO_FILL_LYR_INFO:
        {
            LyricsProperties &prop = g_currentLyrics.properties();

            // Auto fill artist, album and title info
            prop.title = g_player.getTitle();
            prop.artist = g_player.getArtist();
            prop.album = g_player.getAlbum();
            if (g_player.getMediaLength() > 0) {
                prop.setMediaLength(g_player.getMediaLength() / 1000);
            }
            string strLogName = g_profile.getString("LoginName", "");
            if (strLogName.size()) {
                prop.by = strLogName;
            }

            updateLyricsProperties(true);
        }
        break;
    case ID_REMOVE_ALL_TAG:
        {
            int nLineCount;
            string strLine, strTimeTag;

            nLineCount = getLineCount();

            AutoBatchUndo autoBatchUndo(this);
            for (int i = 0; i < nLineCount; i++) {
                getTextOfLine(i, strLine);

                if (!getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    continue;
                }

                setSelOfLine(i, 0, (int)strTimeTag.size());
                removeSel();
            }
        }
        break;
    case ID_REMOVE_BLANK_LINE:
        {
            string strLine;

            AutoBatchUndo autoBatchUndo(this);
            for (int i = getLineCount() - 1; i >= 0; i--) {
                getTextOfLine(i, strLine);

                cstr_t szPtr = strLine.c_str();
                while (*szPtr == ' ' || *szPtr == '\t') {
                    szPtr++;
                }
                if (*szPtr != '\0') {
                    continue;
                }

                if (i == getLineCount() - 1) {
                    setSel(i, 0, i, (int)strLine.size());
                } else {
                    setSel(i, 0, i + 1, 0);
                }
                removeSel();
            }
        }
        break;
    case ID_TRIM_WHITESPACE:
        {
            int nLineCount;
            string strLine, strTimeTag;
            cstr_t szPtr;

            nLineCount = getLineCount();

            AutoBatchUndo autoBatchUndo(this);
            for (int i = 0; i < nLineCount; i++) {
                getTextOfLine(i, strLine);

                szPtr = strLine.c_str();
                // trim leading.
                while (*szPtr == ' ' || *szPtr == '\t') {
                    szPtr++;
                }
                if (szPtr != strLine.c_str()) {
                    setSelOfLine(i, 0, (int)(szPtr - strLine.c_str()));
                    removeSel();
                }

                getTextOfLine(i, strLine);
                // trim leading after time-tag
                if (getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    szPtr = strLine.c_str() + strTimeTag.size();

                    while (*szPtr == ' ' || *szPtr == '\t') {
                        szPtr++;
                    }
                    if (szPtr != strLine.c_str() + strTimeTag.size()) {
                        setSelOfLine(i, (int)strTimeTag.size(), (int)(szPtr - strLine.c_str() - strTimeTag.size()));
                        removeSel();
                    }
                }

                getTextOfLine(i, strLine);
                szPtr = strLine.c_str() + strLine.size() - 1;
                // trim trailing
                while (szPtr != strLine.c_str() && (*szPtr == ' ' || *szPtr == '\t')) {
                    szPtr--;
                }
                if (*szPtr != ' ' && *szPtr != '\t') {
                    szPtr++;
                }
                if (szPtr != strLine.c_str() + strLine.size()) {
                    setSelOfLine(i, (int)(szPtr - strLine.c_str()), (int)(strLine.c_str() + strLine.size() - szPtr));
                    removeSel();
                }
            }
        }
        break;
    case ID_REMOVE_UNSYNC_LINES:
        {
            string strLine, strTimeTag;
            CLyrOneLine *pLine;

            AutoBatchUndo autoBatchUndo(this);
            for (int i = getLineCount() - 1; i >= 0; i--) {
                getTextOfLine(i, strLine);
                pLine = getLine(i);

                if (pLine && !pLine->bTimeLine && !getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    if (i == getLineCount() - 1) {
                        setSel(i, 0, i, (int)strLine.size());
                    } else {
                        setSel(i, 0, i + 1, 0);
                    }
                    removeSel();
                }
            }
        }
        break;
    case ID_CAPITALIZE_LEADING_LETTER:
        {
            string strLine, strTimeTag;
            CLyrOneLine *pLine;
            char szReplaceWith[32];

            memset(szReplaceWith, 0, sizeof(szReplaceWith));

            AutoBatchUndo autoBatchUndo(this);
            for (int i = getLineCount() - 1; i >= 0; i--) {
                getTextOfLine(i, strLine);
                pLine = getLine(i);
                if (!pLine) {
                    continue;
                }

                if (pLine->bTimeLine && getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
                    cstr_t szPos = strLine.c_str() + strTimeTag.size();
                    while (*szPos == ' ' || *szPos == '\t' || *szPos == '\'' || *szPos == '\"') {
                        szPos++;
                    }
                    if (*szPos == '(' || *szPos == '[') {
                        szPos++;
                    }
                    if ((unsigned)(*szPos) <= 255 && islower(*szPos)) {
                        szReplaceWith[0] = toupper(*szPos);
                        int nPos = int(szPos - strLine.c_str());
                        setSel(i, nPos, i, nPos + 1);
                        replaceSel(szReplaceWith);
                    }
                } else {
                    cstr_t szPos = strLine.c_str();
                    while (*szPos == ' ' || *szPos == '\t' || *szPos == '\'' || *szPos == '\"') {
                        szPos++;
                    }
                    if ((unsigned)(*szPos) <= 255 && islower(*szPos)) {
                        szReplaceWith[0] = toupper(*szPos);
                        int nPos = int(szPos - strLine.c_str());
                        setSel(i, nPos, i, nPos + 1);
                        replaceSel(szReplaceWith);
                    }
                }
            }
        }
        break;
    case ID_LYRICS_TO_LOWERCASE:
        {
            string strLine, strTimeTag;

            AutoBatchUndo autoBatchUndo(this);
            for (int i = getLineCount() - 1; i >= 0; i--) {
                getTextOfLine(i, strLine);

                strTimeTag.resize(0);
                getTimeTagOfLine(strLine.c_str(), strTimeTag);

                char * szPos = (char *)strLine.c_str() + strTimeTag.size();
                bool bConverted = false;
                while (*szPos) {
                    if (isupper(*szPos)) {
                        bConverted = true;
                        *szPos = tolower(*szPos);
                    }
                    szPos++;
                }
                if (bConverted) {
                    setSel(i, 0, i, (int)strLine.size());
                    replaceSel(strLine.c_str());
                }
            }
        }
        break;
    case ID_EDIT_LYR_TAG:
        {
            CUIObject *pEditorFrame = m_pSkin->getUIObjectById(getIDByName("CID_EDITOR_FRAME"));
            CUIObject *pTagEditor = m_pSkin->getUIObjectById(getIDByName("CID_C_LYR_TAG"));
            if (!pTagEditor || !pEditorFrame) {
                break;
            }

            bool bVisible = !pTagEditor->isVisible();
            pTagEditor->setVisible(bVisible);
            g_profile.writeInt("LyrTagEdit", bVisible);

            pEditorFrame->getParent()->recalculateUIObjSizePos(pEditorFrame);
            pEditorFrame->invalidate();
        }
        break;
    default:
        return false;
    }
    return true;
}

void CLyricShowTextEditObj::onSaveLyrics() {
    string strEdit;

    getText(strEdit);

    string strId = g_currentLyrics.properties().id;
    g_currentLyrics.properties().clear();
    g_currentLyrics.fromString(strEdit.c_str());

#define SET_PROP(strProp, propCtrlId)                \
    str = m_pSkin->getUIObjectText(propCtrlId);        \
    if (str.size())                                    \
        strProp = str;                                \
    else if (strcmp(strProp.c_str(), str.c_str()) != 0) \
        m_pSkin->setUIObjectText(propCtrlId, strProp.c_str());

    LyricsProperties &prop = g_currentLyrics.properties();
    string str, strOffset = prop.getOffsetTimeStr();
    SET_PROP(prop.artist, CID_E_ARTIST);
    SET_PROP(prop.title, CID_E_TITLE);
    SET_PROP(prop.album, CID_E_ALBUM);
    SET_PROP(prop.by, CID_E_BY);
    SET_PROP(strOffset, CID_E_OFFSET);
    SET_PROP(prop.mediaLength, CID_E_MEDIA_LENGTH);
    if (strOffset.size()) {
        prop.setOffsetTime(strOffset.c_str());
        g_currentLyrics.setOffsetTime(atoi(strOffset.c_str()));
    }
    prop.id = strId;
};

void CLyricShowTextEditObj::onLyricsChanged() {
    if (m_bDiscardLyrChangeEvent) {
        return;
    }

    string str = g_currentLyrics.toString(false);

    updateLyricsProperties();

    setText(str.c_str());

    int nCurLine = g_currentLyrics.getCurPlayLine(g_currentLyrics.getLyricsLines());
    setCaret(nCurLine, 0);
    // ::sendMessage(getHandle(), EM_LINESCROLL, 0, nCurLine);

    m_pSkin->invalidateRect();

    auto pToolBar = (CSkinToolbar *)m_pSkin->getUIObjectById(ID_TB_LYR_EDIT, CSkinToolbar::className());
    if (pToolBar) {
        pToolBar->enableBt(ID_EDIT_REDO, canRedo());
        pToolBar->enableBt(ID_EDIT_UNDO, canUndo());
        pToolBar->enableBt(ID_EDIT_CUT, isSelected());
        pToolBar->enableBt(ID_EDIT_COPY, isSelected());
        pToolBar->invalidate();
    }
}

void CLyricShowTextEditObj::onPlayTimeChangedUpdate() {
    if (m_pSkin->isIconic() || !isVisible() || !isParentVisible()) {
        return;
    }

    {
        // Prevent player forward to next track, if lyrics editor is focus, and lyrics changed.
        if ((int)g_player.getMediaLength() > 30 * 1000
            && canUndo() && g_currentLyrics.getPlayElapsedTime() >= (int)g_player.getMediaLength() - 1000
            && strcmp(g_player.getSrcMedia(), g_currentLyrics.getMediaSource()) == 0) {
            g_player.seekTo(0);
        }
    }

    bool bSelectedEmpty = true;
    if (m_nBegSelRow != -1) {
        bSelectedEmpty = false;
        int nBegSelRow, nEndSelRow;
        int nBegSelCol, nEndSelCol;
        sortSelectPos(nBegSelRow, nBegSelCol, nEndSelRow, nEndSelCol, bSelectedEmpty);
    }

    if (bSelectedEmpty) {
        CRect rcUpdate = getFastDrawRect();
        if (!rcUpdate.empty()) {
            m_pSkin->invalidateRect(&rcUpdate);
        }
    }
}

void CLyricShowTextEditObj::updateLyricsProperties(bool bRedraw) {
    LyricsProperties &prop = g_currentLyrics.properties();

    m_pSkin->setUIObjectText(CID_E_ARTIST, prop.artist.c_str(), false);
    m_pSkin->setUIObjectText(CID_E_TITLE, prop.title.c_str(), false);
    m_pSkin->setUIObjectText(CID_E_ALBUM, prop.album.c_str(), false);
    m_pSkin->setUIObjectText(CID_E_BY, prop.by.c_str(), false);
    m_pSkin->setUIObjectText(CID_E_OFFSET, prop.getOffsetTimeStr().c_str(), false);
    m_pSkin->setUIObjectText(CID_E_MEDIA_LENGTH, prop.mediaLength.c_str(), false);

    if (bRedraw) {
        m_pSkin->invalidateRect();
    }
}

void CLyricShowTextEditObj::onEditorTextChanged(IEditNotification::Status status, bool bVal) {
    CSkinToolbar *pToolBar;

    pToolBar = (CSkinToolbar *)m_pSkin->getUIObjectById(ID_TB_LYR_EDIT, CSkinToolbar::className());
    if (pToolBar) {
        if (status == IEditNotification::S_CAN_REDO) {
            pToolBar->enableBt(ID_EDIT_REDO, bVal);
        }
        if (status == IEditNotification::S_CAN_UNDO) {
            pToolBar->enableBt(ID_EDIT_UNDO, bVal);
        }
        if (status == IEditNotification::S_SEL) {
            pToolBar->enableBt(ID_EDIT_CUT, bVal);
            pToolBar->enableBt(ID_EDIT_COPY, bVal);
        }
        pToolBar->invalidate();
    }
}

void CLyricShowTextEditObj::onEditorTextChanged() {
    if (m_nTimerIDUpdateLyrics) {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDUpdateLyrics);
    }
    m_nTimerIDUpdateLyrics = m_pSkin->registerTimerObject(this, TIMER_SPAN_LYR_PREVIEW_UPDATE);
    //     m_pSkin->killTimer(TIMER_LYR_PREVIEW_UPDATE);
    //     m_pSkin->setTimer(TIMER_LYR_PREVIEW_UPDATE, TIMER_SPAN_LYR_PREVIEW_UPDATE);
}

void CLyricShowTextEditObj::onTimer(int nId) {
    if (nId == m_nTimerIDUpdateLyrics) {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDUpdateLyrics);

        onSaveLyrics();

        m_bDiscardLyrChangeEvent = true;
        MPlayerApp::getInstance()->dispatchLyricsChangedSyncEvent();
        m_bDiscardLyrChangeEvent = false;
    } else {
        CSkinEditCtrl::onTimer(nId);
    }
}

void CLyricShowTextEditObj::updateTextFontColor() {
    CColor m_clrTag, m_clrFocusLineBg;
    CColor m_clrLowlight, m_clrHighlight, m_clrBg;

    profileGetColorValue(m_clrHighlight, SZ_SECT_LYR_DISPLAY, "Ed_HighColor");
    profileGetColorValue(m_clrLowlight, SZ_SECT_LYR_DISPLAY, "Ed_LowColor");
    profileGetColorValue(m_clrBg, SZ_SECT_LYR_DISPLAY, "Ed_BgColor");
    profileGetColorValue(m_clrTag, SZ_SECT_LYR_DISPLAY, "Ed_TagColor");
    profileGetColorValue(m_clrFocusLineBg, SZ_SECT_LYR_DISPLAY, "Ed_FocusLineBgColor");

    setColor(CN_BG, m_clrBg);
    setSelColor(m_clrHighlight, m_clrFocusLineBg);
    setColorTheme(m_clrHighlight, m_clrLowlight, m_clrTag, m_clrFocusLineBg);

    const int nEditorFontMaxSize = 22;

    FontInfoEx info;
    profileGetLyricsFont(SZ_SECT_LYR_DISPLAY, info);
    if (info.height > nEditorFontMaxSize) {
        info.height = nEditorFontMaxSize;
    }

    m_font.create(info);

    onFontChanged();
}

void CLyricShowTextEditObj::syncTimeTag(bool bMoveToNextLine) {
    int nBegLine, nEndLine, nCol = 0;
    int nBegCol = 0, nEndCol = 0;
    bool bSelectedEmpty = true;
    string strLine, strTimeTag;
    int nTime;

    if (m_nBegSelRow != -1) {
        sortSelectPos(nBegLine, nBegCol, nEndLine, nEndCol, bSelectedEmpty);
    } else {
        getCaret(nBegLine, nCol);
        nEndLine = nBegLine;
    }

    int nTimeDuarationOffset = 0;
    int nLine;
    AutoBatchUndo autoBatchUndo(this);
    int nPlayingPos = g_currentLyrics.getPlayElapsedTime();
    if (nPlayingPos < 0) {
        nPlayingPos = 0;
    }

    for (nLine = nBegLine; nLine <= nEndLine; nLine++) {
        if (!getTextOfLine(nLine, strLine)) {
            continue;
        }

        if (!getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
            // This line wasn't synchronized yet,
            if (nLine == nBegLine) {
                string newTimeTag = formatLrcTimeTag(nPlayingPos, true);

                setSelOfLine(nLine, 0, 0);
                replaceSel(newTimeTag.c_str());
                break;
            }

            continue;
        }

        // Adjust the time of this line.
        nTime = getTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
        if (nLine == nBegLine) {
            nTimeDuarationOffset = nTime - nPlayingPos;
            nTime = g_currentLyrics.getPlayElapsedTime();
        } else {
            nTime -= nTimeDuarationOffset;
        }

        // set new time
        string newTimeTag = formatLrcTimeTag(nTime, true);

        setSelOfLine(nLine, 0, (int)strTimeTag.size());
        replaceSel(newTimeTag.c_str());
    }

    if (!bSelectedEmpty) {
        setSel(nBegLine, nBegCol, nEndLine, nEndCol);
        return;
    }

    if (bMoveToNextLine && nBegLine == nEndLine) {
        setCaret(nEndLine + 1, 0);
        autoVScrollDown(1);
    } else {
        setCaret(nEndLine, nCol);
    }
}

void CLyricShowTextEditObj::adjustSyncTimeOfSelected(bool bIncreaseTime) {
    int nBegLine, nEndLine, nCol;
    string strLine, strTimeTag;
    int nTime;
    int nBegCol = 0, nEndCol = 0;
    bool bSelectedEmpty = true;

    if (m_nBegSelRow != -1) {
        sortSelectPos(nBegLine, nBegCol, nEndLine, nEndCol, bSelectedEmpty);
    } else {
        getCaret(nBegLine, nCol);
        nEndLine = nBegLine;
    }

    int nLine;
    AutoBatchUndo autoBatchUndo(this);

    for (nLine = nBegLine; nLine <= nEndLine; nLine++) {
        if (!getTextOfLine(nLine, strLine)) {
            continue;
        }

        if (!getTimeTagOfLine(strLine.c_str(), strTimeTag)) {
            // This line wasn't synchronized yet,
            continue;
        }

        // Adjust the time of this line.
        nTime = getTimeTagValue(strTimeTag.c_str(), strTimeTag.size());
        if (bIncreaseTime) {
            nTime += 200;
        } else {
            nTime -= 200;
        }

        // set new time
        string newTimeTag = formatLrcTimeTag(nTime, true);

        setSelOfLine(nLine, 0, (int)strTimeTag.size());
        replaceSel(newTimeTag.c_str());
    }

    if (!bSelectedEmpty) {
        setSel(nBegLine, nBegCol, nEndLine, nEndCol);
    }
}

void CLyricShowTextEditObj::showFindDialog(bool bFind) {
    CUIObject *pFind = m_pSkin->getUIObjectById(getIDByName("CID_C_FIND"));
    CUIObject *pReplace = m_pSkin->getUIObjectById(getIDByName("CID_C_REPLACE"));
    CUIObject *pEditorFrame = m_pSkin->getUIObjectById(getIDByName("CID_EDITOR_FRAME"));

    if (!pFind || !pReplace || !pEditorFrame) {
        return;
    }

    if ((bFind && m_findReplaceStatus == FRS_FIND)
        || (!bFind && m_findReplaceStatus == FRS_REPLACE)) {
        // Hide
        m_findReplaceStatus = FRS_NONE;
        pFind->setVisible(false, false);
        pReplace->setVisible(false, false);
        pEditorFrame->getParent()->recalculateUIObjSizePos(pEditorFrame);
        pEditorFrame->invalidate();
        return;
    }

    if (m_findReplaceStatus == FRS_NONE) {
        string strFindWhat;
        selectionToText(strFindWhat);
        if (strFindWhat.size()) {
            m_pSkin->setUIObjectText(CID_E_FIND, strFindWhat.c_str(), false);
        }
    }

    // show
    if (bFind) {
        m_findReplaceStatus = FRS_FIND;
        pFind->setVisible(true, false);
        pReplace->setVisible(false, false);
    } else {
        m_findReplaceStatus = FRS_REPLACE;
        pFind->setVisible(true, false);
        pReplace->setVisible(true, false);
    }

    pEditorFrame->getParent()->recalculateUIObjSizePos(pEditorFrame);
    m_pSkin->invalidateRect();
}

bool CLyricShowTextEditObj::findNext(bool bSearchDown, bool isMessageOut) {
    uint32_t dwFlags = 0;

    if (m_pSkin->isButtonChecked(CID_MATCHCASE)) {
        dwFlags |= FTF_MATCH_CASE;
    }
    if (bSearchDown) {
        dwFlags |= FTF_DOWN;
    }

    string strFindWhat = m_pSkin->getUIObjectText(CID_E_FIND);

    if (!findText(strFindWhat.c_str(), dwFlags)) {
        if (isMessageOut) {
            m_pSkin->messageOut(stringPrintf(_TLT("Can't find \"%s\""), strFindWhat.c_str()).c_str());
        }
        return false;
    }

    return true;
}
