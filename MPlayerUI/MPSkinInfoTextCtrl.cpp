#include "MPlayerAppBase.h"
#include "MPSkinInfoTextCtrl.h"


UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrl, "InfoText")

CMPSkinInfoTextCtrl::CMPSkinInfoTextCtrl() {
    m_msgNeed = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE;
    m_nTimerIDHideInfo = 0;
    m_dwAlignText = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
}

CMPSkinInfoTextCtrl::~CMPSkinInfoTextCtrl() {
    if (m_nTimerIDHideInfo != 0) {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDHideInfo);
    }
}

void CMPSkinInfoTextCtrl::onCreate() {
    CSkinScrollText::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_UI_INFO_TEXT, ET_UI_LONG_ERROR_TEXT);
    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_LYRICS_CHANGED);

    updateShowDefaultInfo();
    m_cursor.loadStdCursor(Cursor::C_HAND);
}

void CMPSkinInfoTextCtrl::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_UI_INFO_TEXT) {
        setText(pEvent->strValue.c_str());
        m_strCmd.clear();
        invalidate();
        m_pSkin->unregisterTimerObject(this);
        m_nTimerIDHideInfo = m_pSkin->registerTimerObject(this, 5000);
        setVisible(true, true);
    } else if (pEvent->eventType == ET_UI_LONG_ERROR_TEXT) {
        setText(pEvent->strValue.c_str());
        m_strCmd = pEvent->name;
        invalidate();
        m_pSkin->unregisterTimerObject(this);
        m_nTimerIDHideInfo = m_pSkin->registerTimerObject(this, 30000);
        setVisible(true, true);
    } else if (pEvent->eventType == ET_LYRICS_CHANGED) {
        updateShowDefaultInfo();
    }
}

bool CMPSkinInfoTextCtrl::onMouseMove(CPoint point) {
    if (m_strCmd.empty()) {
        return false;
    }

    setCursor(m_cursor);

    return true;
}

bool CMPSkinInfoTextCtrl::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (m_strCmd.empty()) {
        return false;
    }

    cstr_t SZ_CMD = "cmd://";
    if (startsWith(m_strCmd.c_str(), SZ_CMD)) {
        string strIDs = m_strCmd.c_str() + strlen(SZ_CMD);
        VecStrings vIDs;
        strSplit(strIDs.c_str(), '|', vIDs);

        if (vIDs.size() > 1) {
            CSkinMenu m_menu;

            m_menu.createPopupMenu();
            for (uint32_t i = 0; i < vIDs.size(); i++) {
                int nId = m_pSkin->getSkinFactory()->getIDByName(vIDs[i].c_str());
                string tooltip = m_pSkin->getSkinFactory()->getTooltip(nId);
                m_menu.appendItem(nId, tooltip.c_str());
            }

            CPoint pt = getCursorPos();
            m_menu.trackPopupMenu(pt.x, pt.y, m_pSkin);
        } else {
            int nId = m_pSkin->getSkinFactory()->getIDByName(vIDs[0].c_str());
            m_pSkin->postCustomCommandMsg(nId);
        }
    } else if (startsWith(m_strCmd.c_str(), "http://")) {
        openUrl(m_pSkin, m_strCmd.c_str());
    }

    setCursor(m_cursor);

    return true;
}

bool CMPSkinInfoTextCtrl::onLButtonDown(uint32_t nFlags, CPoint point) {
    if (m_strCmd.empty()) {
        return false;
    }

    setCursor(m_cursor);

    return true;
}

void CMPSkinInfoTextCtrl::onTimer(int nId) {
    if (nId == m_nTimerIDHideInfo) {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDHideInfo);
        m_nTimerIDHideInfo = 0;
        updateShowDefaultInfo();
        invalidate();
    } else {
        CSkinScrollText::onTimer(nId);
    }
}

void CMPSkinInfoTextCtrl::draw(CRawGraph *canvas) {
    if (isFlagSet(m_dwAlignText, AT_CENTER)) {
        CSkinStaticText::draw(canvas);
    } else {
        CSkinScrollText::draw(canvas);
    }
}

void CMPSkinInfoTextCtrl::updateShowDefaultInfo() {
    // Hide itself.
    setVisible(false, true);
    // setText(g_player.getFullTitle());
}
