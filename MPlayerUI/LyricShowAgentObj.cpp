// LyricShowAgentObj.cpp: implementation of the CLyricShowAgentObj class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "LyricShowAgentObj.h"
#include "MPSkinInfoTextCtrl.h"

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CLyricShowAgentObj, "LyricsShow")

CLyricShowAgentObj::CLyricShowAgentObj()
{
    m_pLyricsShow = nullptr;
    m_bEnableStaticTextStyle = true;

    m_strLyrDisplayStylePropName = "LyrDisplayStyle";
    m_bEnableToolbar = true;
    m_pToolbar = nullptr;
    m_bFloatingLyr = false;
    m_pInfoTextCtrl = nullptr;
}

CLyricShowAgentObj::~CLyricShowAgentObj()
{
}

void CLyricShowAgentObj::getLyrDispStylePropName(CSkinWnd *pSkinWnd, bool &bFloatingLyr, string &strLyrStylePropName)
{
    bFloatingLyr = false;

    if (!pSkinWnd->getUnprocessedProperty("LyrDisplayStylePropName", strLyrStylePropName))
        strLyrStylePropName = "LyrDisplayStyle";

    if (strcasecmp("FloatingLyrDispStyle", strLyrStylePropName.c_str()) == 0)
    {
        bFloatingLyr = true;
        strLyrStylePropName = "LyrDisplayStyle";
    }
}

void CLyricShowAgentObj::getLyrDispStyleSettings(CSkinWnd *pSkinWnd, string &strLyrStyle)
{
    string            strLyrStylePropName;
    bool            bFloatingLyr;

    getLyrDispStylePropName(pSkinWnd, bFloatingLyr, strLyrStylePropName);

    strLyrStyle = g_profile.getString(
        CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr),
        strLyrStylePropName.c_str(),
        strLyrStyle.c_str());
}

void CLyricShowAgentObj::setLyrDispStyleSettings(CSkinWnd *pSkinWnd, cstr_t szLyrStyle)
{
    string            strLyrStylePropName;
    bool            bFloatingLyr;

    getLyrDispStylePropName(pSkinWnd, bFloatingLyr, strLyrStylePropName);

    CMPlayerSettings::setSettings(ET_UI_SETTINGS_CHANGED, 
        CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr), 
        strLyrStylePropName.c_str(), szLyrStyle);
}

void CLyricShowAgentObj::onCreate()
{
    CSkinLinearContainer::onCreate();

    getLyrDispStylePropName(m_pSkin, m_bFloatingLyr, m_strLyrDisplayStylePropName);

    m_strLyrDisplayStyleDefault = g_profile.getString(
        CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(m_bFloatingLyr),
        m_strLyrDisplayStylePropName.c_str(),
        m_strLyrDisplayStyleDefault.c_str());

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_UI_SETTINGS_CHANGED, ET_LYRICS_CHANGED);

    if (m_bEnableToolbar && !g_profile.getBool("HideToolbar", false))
    {
        // create tool bar
        loadToolbar();
    }

    changeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str(), false);

    createInfoTextCtrl();
}

void CLyricShowAgentObj::onEvent(const IEvent *pEvent)
{
    if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
    {
        if (strcasecmp(pEvent->name.c_str(), m_strLyrDisplayStylePropName.c_str()) == 0)
        {
            getLyrDispStyleSettings(m_pSkin, m_strLyrDisplayStyleDefault);
            changeLyricsDisplayStyle(m_strLyrDisplayStyleDefault.c_str());
            m_pSkin->invalidateRect();
        }
        else if (isPropertyName(pEvent->name.c_str(), "HideToolbar"))
        {
            bool        bHideToolbar;

            bHideToolbar = isTRUE(pEvent->strValue.c_str());
            if (bHideToolbar)
            {
                if (m_pToolbar)
                {
                    // destroy toolbar
                    this->removeUIObject(m_pToolbar, true);
                    m_pToolbar = nullptr;
                }
            }
            else
                loadToolbar();

            m_pContainer->recalculateUIObjSizePos(this);
        }
    }
    else if (pEvent->eventType == ET_LYRICS_CHANGED && m_bEnableStaticTextStyle)
    {
        m_pSkin->postExecOnMainThread([this]() {
            string        strNewStyle;
            if (g_LyricData.getLyrContentType() != LCT_TXT)
                strNewStyle = m_strLyrDisplayStyleDefault;
            else
                strNewStyle = SZ_TXT_LYR_CONTAINER;

            changeLyricsDisplayStyle(strNewStyle.c_str());

            m_pContainer->recalculateUIObjSizePos(this);
        });
    }
}

bool CLyricShowAgentObj::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (m_pLyricsShow)
        m_pLyricsShow->setProperty(szProperty, szValue);

    if (CSkinLinearContainer::setProperty(szProperty, szValue))
        return true;

    if (isPropertyName(szProperty, "LyrDisplayStyle")
        || isPropertyName(szProperty, m_strLyrDisplayStylePropName.c_str()))
        m_strLyrDisplayStyleDefault = szValue;
    else if (isPropertyName(szProperty, "EnableStaticTextStyle"))
        m_bEnableStaticTextStyle = isTRUE(szValue);
    else if (isPropertyName(szProperty, "EnableToolbar"))
        m_bEnableToolbar = isTRUE(szValue);
    else if (isPropertyName(szProperty, "FgColor"))
    {
        if (m_pInfoTextCtrl && m_pInfoTextCtrl->isUseParentBg())
            m_pInfoTextCtrl->setProperty("TextColor", szValue);
    }
    else
    {
        m_vProperties.push_back(szProperty);
        m_vProperties.push_back(szValue);
    }

    return true;
}

void CLyricShowAgentObj::createInfoTextCtrl()
{
    // Do NOT display activation notice, if there's no lyrics opened.
    if (m_pInfoTextCtrl)
    {
        m_pInfoTextCtrl->setVisible(true, false);
    }
    else
    {
        if (!isPropertyName(m_strLyrDisplayStylePropName.c_str(), "LyrDisplayStyle")
            || m_bFloatingLyr || m_pSkin->m_bClickThrough)
            return;

        m_pInfoTextCtrl = (CMPSkinInfoTextCtrl*)m_pSkin->getSkinFactory()->createUIObject(m_pSkin, 
            CMPSkinInfoTextCtrl::className(), this);
        assert(m_pInfoTextCtrl);
        if (m_pInfoTextCtrl->isUseParentBg())
            m_pInfoTextCtrl->setProperty("TextColor", colorToStr(m_pLyricsShow->getHighlightColor()).c_str());
        m_pInfoTextCtrl->setProperty(SZ_PN_RECT, "0,0,w,");

        this->addUIObject(m_pInfoTextCtrl);
    }
}

void CLyricShowAgentObj::changeLyricsDisplayStyle(cstr_t szLyrDispalyStyle, bool bRedraw)
{
    if (strcasecmp(m_strLyrDisplayStyleCurrent.c_str(), szLyrDispalyStyle) == 0
        && !isEmptyString(szLyrDispalyStyle))
        return;

    if (m_pLyricsShow)
    {
        m_pSkin->removeUIObject(m_pLyricsShow, true);
        m_pLyricsShow = nullptr;
    }

    if ((strcasecmp(szLyrDispalyStyle, CLyricShowMultiRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, SZ_TXT_LYR_CONTAINER) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowTwoRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowSingleRowObj::className()) != 0
        && strcasecmp(szLyrDispalyStyle, CLyricShowVobSub::className()) != 0))
    {
        m_strLyrDisplayStyleDefault = CLyricShowMultiRowObj::className();
        szLyrDispalyStyle = m_strLyrDisplayStyleDefault.c_str();
    }

    m_strLyrDisplayStyleCurrent = szLyrDispalyStyle;

    m_pLyricsShow = (CLyricShowObj*)m_pSkin->getSkinFactory()->createUIObject(m_pSkin, szLyrDispalyStyle, this);
    if (!m_pLyricsShow)
        return;

    m_pLyricsShow->setProperty(SZ_PN_RECT, "0,0,w,0");
    m_pLyricsShow->setProperty(SZ_PN_WEIGHT, "1");
    m_pLyricsShow->setProperties(m_vProperties);

    this->addUIObject(m_pLyricsShow);

    // Switch to proper lyrics toolbar
    CUIObject *pToolbarLyrSync = m_pSkin->getUIObjectById(ID_TB_LYR_SYNC);
    CSkinToolbar *pToolbarLyrTxt = (CSkinToolbar*)m_pSkin->getUIObjectById(ID_TB_LYR_TXT, CSkinToolbar::className());
    if (pToolbarLyrSync && pToolbarLyrTxt)
    {
        bool bTxtStyle = (strcasecmp(m_strLyrDisplayStyleCurrent.c_str(), SZ_TXT_LYR_CONTAINER) == 0);
        if (bTxtStyle)
        {
            CLyricShowTxtObj *pTxtObj = (CLyricShowTxtObj *)m_pSkin->getUIObjectByClassName(CLyricShowTxtObj::className());
            if (pTxtObj)
            {
                pToolbarLyrTxt->setCheck(CMD_LYR_SCROLL_ENABLE_RECORD, 
                    pTxtObj->isRecordScrollingActionsEnabled() && g_LyricData.getLyrContentType() == LCT_TXT, false);
                pToolbarLyrTxt->setCheck(CMD_LYR_SCROLL_ENABLE_REPLAY, pTxtObj->isReplayScrollingActionsEnabled(), false);
            }
        }

        pToolbarLyrSync->setVisible(!bTxtStyle, true);
        pToolbarLyrTxt->setVisible(bTxtStyle, true);
    }
}

void CLyricShowAgentObj::loadToolbar()
{
    if (!m_bEnableToolbar)
        return;

    if (m_pToolbar)
        this->removeUIObject(m_pToolbar, true);

    m_pToolbar = m_pSkin->getSkinFactory()->createUIObject(m_pSkin, "NormalToolbar", this);
    if (m_pToolbar)
    {
        m_pToolbar->setProperty(SZ_PN_RECT, "0,0,w,");
        this->insertUIObjectAt(m_pLyricsShow, m_pToolbar);
    }
}
