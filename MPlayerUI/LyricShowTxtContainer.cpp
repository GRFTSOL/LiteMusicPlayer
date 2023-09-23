#include "MPlayerApp.h"
#include "LyricShowTxtContainer.h"
#include "LyricShowTxtObj.h"


UIOBJECT_CLASS_NAME_IMP(CLyricShowTxtContainer, "LyricShowTxtContainer")

CLyricShowTxtContainer::CLyricShowTxtContainer() {
    m_bVertical = false;
    m_pLyricsShow = nullptr;
    m_pObjScrollBar = nullptr;
}

CLyricShowTxtContainer::~CLyricShowTxtContainer() {
}

void CLyricShowTxtContainer::onCreate() {
    CSkinLinearContainer::onCreate();

    //
    // create Scrollbar
    //
    m_pObjScrollBar = m_pSkin->createUIObject(CSkinVScrollBar::className(), m_pContainer);
    if (!m_pObjScrollBar) {
        return;
    }

    m_pObjScrollBar->setProperty("LinesPerWheel", "1");
    m_pObjScrollBar->setProperty(SZ_PN_LAYOUT_PARAMS, "match_parent_height");

    addUIObject(m_pObjScrollBar);


    m_pLyricsShow = m_pSkin->createUIObject(CLyricShowTxtObj::className(), this);
    if (!m_pLyricsShow) {
        return;
    }

    m_pLyricsShow->setProperty(SZ_PN_RECT, "0,0,0,h");
    m_pLyricsShow->setProperty(SZ_PN_WEIGHT, "1");
    m_pLyricsShow->setProperties(m_vProperties);

    insertUIObjectAt(m_pObjScrollBar, m_pLyricsShow);
}

bool CLyricShowTxtContainer::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (m_pLyricsShow) {
        m_pLyricsShow->setProperty(szProperty, szValue);
    }

    if (CSkinLinearContainer::setProperty(szProperty, szValue)) {
        return true;
    }

    m_vProperties.push_back(szProperty);
    m_vProperties.push_back(szValue);

    return true;
}
