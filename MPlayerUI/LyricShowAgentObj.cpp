#include "MPlayerApp.h"
#include "LyricShowAgentObj.h"
#include "MPSkinInfoTextCtrl.h"


#define SZ_LYR_DISPLAY_STYLE "LyrDisplayStyle"

UIOBJECT_CLASS_NAME_IMP(CLyricShowAgentObj, "LyricsShow")

CLyricShowAgentObj::CLyricShowAgentObj() {
    m_pLyricsShow = nullptr;
    m_bEnableStaticTextStyle = true;

    m_bEnableToolbar = true;
    m_pToolbar = nullptr;
    m_bFloatingLyr = false;
    m_pInfoTextCtrl = nullptr;
}

CLyricShowAgentObj::~CLyricShowAgentObj() {
}

bool CLyricShowAgentObj::isFloatingLyrMode(CSkinWnd *pSkinWnd) {
    string value;
    return pSkinWnd->getUnprocessedProperty("IsFloatingLyrics", value) &&
        isTRUE(value.c_str());
}

void CLyricShowAgentObj::getLyrDispStyleSettings(CSkinWnd *pSkinWnd, string &strLyrStyle) {
    bool isFloatingLyr = isFloatingLyrMode(pSkinWnd);

    strLyrStyle = g_profile.getString(
        MPlayerApp::getInstance()->getCurLyrDisplaySettingName(isFloatingLyr),
        SZ_LYR_DISPLAY_STYLE,
        strLyrStyle.c_str());
}

void CLyricShowAgentObj::setLyrDispStyleSettings(CSkinWnd *pSkinWnd, cstr_t szLyrStyle) {
    bool isFloatingLyr = isFloatingLyrMode(pSkinWnd);

    CMPlayerSettings::setSettings(ET_UI_SETTINGS_CHANGED,
        MPlayerApp::getInstance()->getCurLyrDisplaySettingName(isFloatingLyr),
        SZ_LYR_DISPLAY_STYLE, szLyrStyle);
}

void CLyricShowAgentObj::onCreate() {
    CSkinLinearContainer::onCreate();

    m_bFloatingLyr = isFloatingLyrMode(m_pSkin);

    m_strLyrDisplayStyleDefault = g_profile.getString(
        MPlayerApp::getInstance()->getCurLyrDisplaySettingName(m_bFloatingLyr),
        SZ_LYR_DISPLAY_STYLE,
        m_strLyrDisplayStyleDefault.c_str());

    registerHandler(MPlayerApp::getEventsDispatcher(), ET_UI_SETTINGS_CHANGED, ET_LYRICS_CHANGED);

    if (m_bEnableToolbar && !g_profile.getBool("HideToolbar", false)) {
        // create tool bar
        loadToolbar();
    }

    changeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str(), false);

    createInfoTextCtrl();
}

void CLyricShowAgentObj::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_UI_SETTINGS_CHANGED) {
        if (strcasecmp(pEvent->name.c_str(), SZ_LYR_DISPLAY_STYLE) == 0) {
            getLyrDispStyleSettings(m_pSkin, m_strLyrDisplayStyleDefault);
            changeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str());
            m_pSkin->invalidateRect();
        } else if (isPropertyName(pEvent->name.c_str(), "HideToolbar")) {
            bool bHideToolbar;

            bHideToolbar = isTRUE(pEvent->strValue.c_str());
            if (bHideToolbar) {
                if (m_pToolbar) {
                    // destroy toolbar
                    this->removeUIObject(m_pToolbar, true);
                    m_pToolbar = nullptr;
                }
            } else {
                loadToolbar();
            }

            m_pContainer->recalculateUIObjSizePos(this);
        }
    } else if (pEvent->eventType == ET_LYRICS_CHANGED && m_bEnableStaticTextStyle) {
        m_pSkin->postExecOnMainThread([this]() {
            string strNewStyle;
            if (g_currentLyrics.getLyrContentType() != LCT_TXT) {
                strNewStyle = m_strLyrDisplayStyleDefault;
            } else {
                strNewStyle = SZ_TXT_LYR_CONTAINER;
            }

            changeLyricsDisplayStyle(strNewStyle.c_str());

            m_pContainer->recalculateUIObjSizePos(this);
        });
    }
}

bool CLyricShowAgentObj::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (m_pLyricsShow) {
        m_pLyricsShow->setProperty(szProperty, szValue);
    }

    if (CSkinLinearContainer::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "LyrDisplayStyle")) {
        m_strLyrDisplayStyleDefault = szValue;
    } else if (isPropertyName(szProperty, "EnableStaticTextStyle")) {
        m_bEnableStaticTextStyle = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "EnableToolbar")) {
        m_bEnableToolbar = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "FgColor")) {
        if (m_pInfoTextCtrl && m_pInfoTextCtrl->isUseParentBg()) {
            m_pInfoTextCtrl->setProperty("TextColor", szValue);
        }
    } else {
        m_vProperties.push_back(szProperty);
        m_vProperties.push_back(szValue);
    }

    return true;
}

void CLyricShowAgentObj::createInfoTextCtrl() {
    // Do NOT display activation notice, if there's no lyrics opened.
    if (m_pInfoTextCtrl) {
        m_pInfoTextCtrl->setVisible(true, false);
    } else {
        if (m_bFloatingLyr || m_pSkin->m_bClickThrough) {
            return;
        }

        m_pInfoTextCtrl = (CMPSkinInfoTextCtrl*)m_pSkin->createUIObject(CMPSkinInfoTextCtrl::className(), this);
        assert(m_pInfoTextCtrl);
        if (m_pInfoTextCtrl->isUseParentBg()) {
            m_pInfoTextCtrl->setProperty("TextColor", colorToStr(m_pLyricsShow->getHighlightColor()).c_str());
        }
        m_pInfoTextCtrl->setProperty(SZ_PN_RECT, "0,0,w,");

        this->addUIObject(m_pInfoTextCtrl);
    }
}

void CLyricShowAgentObj::changeLyricsDisplayStyle(cstr_t szLyrDispalyStyle, bool bRedraw) {
    if (strcasecmp(m_strLyrDisplayStyleCurrent.c_str(), szLyrDispalyStyle) == 0
        && !isEmptyString(szLyrDispalyStyle)) {
        return;
    }

    if (m_pLyricsShow) {
        m_pSkin->removeUIObject(m_pLyricsShow, true);
        m_pLyricsShow = nullptr;
    }

    if ((strcasecmp(szLyrDispalyStyle, CLyricShowMultiRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, SZ_TXT_LYR_CONTAINER) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowTwoRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowSingleRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowVobSub::className()) != 0)) {
        m_strLyrDisplayStyleDefault = CLyricShowMultiRowObj::className();
        szLyrDispalyStyle = m_strLyrDisplayStyleDefault.c_str();
    }

    m_strLyrDisplayStyleCurrent = szLyrDispalyStyle;

    m_pLyricsShow = (CLyricShowObj*)m_pSkin->createUIObject(szLyrDispalyStyle, this);
    if (!m_pLyricsShow) {
        return;
    }

    m_pLyricsShow->setProperty(SZ_PN_RECT, "0,0,w,0");
    m_pLyricsShow->setProperty(SZ_PN_WEIGHT, "1");
    m_pLyricsShow->setProperties(m_vProperties);

    this->addUIObject(m_pLyricsShow);

    // Switch to proper lyrics toolbar
    CUIObject *pToolbarLyrSync = m_pSkin->getUIObjectById(ID_TB_LYR_SYNC);
    CSkinToolbar *pToolbarLyrTxt = (CSkinToolbar*)m_pSkin->getUIObjectById(ID_TB_LYR_TXT, CSkinToolbar::className());
    if (pToolbarLyrSync && pToolbarLyrTxt) {
        bool bTxtStyle = (strcasecmp(m_strLyrDisplayStyleCurrent.c_str(), SZ_TXT_LYR_CONTAINER) == 0);

        pToolbarLyrSync->setVisible(!bTxtStyle, true);
        pToolbarLyrTxt->setVisible(bTxtStyle, true);

        // 根据当前的模式调整 toolbar 的大小.
        auto parentToolbar = pToolbarLyrSync->getParent();
        int w1 = bTxtStyle ? pToolbarLyrTxt->m_rcObj.width() : pToolbarLyrSync->m_rcObj.width();
        int w2 = !bTxtStyle ? pToolbarLyrTxt->m_rcObj.width() : pToolbarLyrSync->m_rcObj.width();
        parentToolbar->m_rcObj.right -= w2 - w1;
    }
}

void CLyricShowAgentObj::loadToolbar() {
    if (!m_bEnableToolbar) {
        return;
    }

    if (m_pToolbar) {
        this->removeUIObject(m_pToolbar, true);
    }

    m_pToolbar = m_pSkin->createUIObject("NormalToolbar", this);
    if (m_pToolbar) {
        m_pToolbar->setProperty(SZ_PN_RECT, "0,0,w,");
        this->insertUIObjectAt(m_pLyricsShow, m_pToolbar);
    }
}
