#include "MPlayerApp.h"
#include "MPSkinInfoTextCtrlEx.h"


UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrlEx,  "InfoTextEx")

CMPSkinInfoTextCtrlEx::CMPSkinInfoTextCtrlEx() {
}

CMPSkinInfoTextCtrlEx::~CMPSkinInfoTextCtrlEx() {
}

void CMPSkinInfoTextCtrlEx::onCreate() {
    CLyricShowObj::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_UI_INFO_TEXT, ET_UI_LONG_ERROR_TEXT);

    // Do not proceed these messages.
    CMPlayerAppBase::getEventsDispatcher()->unRegisterHandler(ET_PLAYER_CUR_MEDIA_CHANGED, this);
    CMPlayerAppBase::getEventsDispatcher()->unRegisterHandler(ET_LYRICS_CHANGED, this);
    CMPlayerAppBase::getEventsDispatcher()->unRegisterHandler(ET_LYRICS_DRAW_UPDATE, this);
    if (m_bUseBgImg) {
        m_pSkin->unregisterTimerObject(this);
    }
}

void CMPSkinInfoTextCtrlEx::draw(CRawGraph *canvas) {
    canvas->setFont(&m_font);

    int nLineHeight = getLineHeight();
    int y = (m_rcObj.top + m_rcObj.bottom) / 2 - (int)m_vText.size() * nLineHeight / 2;
    int x;

    if (m_vText.size() > 1) {
        x = m_rcObj.left + m_nXMargin;
    } else {
        x = AUTO_CAL_X;
    }

    if (y < m_rcObj.top + m_nYMargin) {
        y = m_rcObj.top + m_nYMargin;
    }

    CRect rcClip;

    rcClip.setLTRB(m_rcObj.left + m_nXMargin,
        m_rcObj.top + m_nYMargin,
        m_rcObj.right - m_nXMargin,
        m_rcObj.bottom - m_nYMargin);

    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    canvas->setClipBoundBox(rcClip);

    for (size_t i = 0; i < m_vText.size(); i++) {
        drawRow(canvas, m_vText[i], x, y, LP_CUR_LINE);
        y += nLineHeight;
    }
}

void CMPSkinInfoTextCtrlEx::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_UI_INFO_TEXT) {
        m_vUnWrapedText.clear();
        appendInfoText(pEvent->strValue);
        wrapInfoText();
        invalidate();
    } else if (pEvent->eventType == ET_UI_LONG_ERROR_TEXT) {
        VecStrings vStr;

        m_vUnWrapedText.clear();
        strSplit(pEvent->strValue.c_str(), '\n', vStr);
        for (size_t i = 0; i < vStr.size(); i++) {
            trimStr(vStr[i], '\r');
            appendInfoText(vStr[i]);
        }

        wrapInfoText();
        invalidate();
    } else {
        CLyricShowObj::onEvent(pEvent);
    }
}

void CMPSkinInfoTextCtrlEx::onLyrDrawContextChanged() {

}

void CMPSkinInfoTextCtrlEx::onPlayTimeChangedUpdate() {

}

void CMPSkinInfoTextCtrlEx::onSize() {
    wrapInfoText();
}

void CMPSkinInfoTextCtrlEx::appendInfoText(const string &str) {
    LyricsLine line(0, 0);
    line.appendPiece(0, 0, str);

    m_vUnWrapedText.push_back(line);
}

bool wrapDisplayLyrics(LyricsLines &lyrLinesSrc, LyricsLines &lyrLinesOut, CRawGraph *canvas, int nWidthMax, bool bVerticalStyle);

void CMPSkinInfoTextCtrlEx::wrapInfoText() {
    CRawGraph *canvas;

    canvas = m_pContainer->getMemGraph();

    canvas->setFont(&m_font);

    m_vText.clear();
    wrapDisplayLyrics(m_vUnWrapedText, m_vText, canvas, m_rcObj.width() - m_nXMargin * 4, true);
}
