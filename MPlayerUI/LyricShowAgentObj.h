#pragma once

#ifndef MPlayerUI_LyricShowAgentObj_h
#define MPlayerUI_LyricShowAgentObj_h


#include "LyricShowObj.h"
#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtObj.h"
#include "LyricShowTwoRowObj.h"
#include "LyricShowSingleRowObj.h"
#include "LyricShowVobSub.h"
#include "../Skin/SkinLinearContainer.h"


class CLyricShowAgentObj : public CSkinLinearContainer, IEventHandler {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinLinearContainer)
public:
    CLyricShowAgentObj();
    virtual ~CLyricShowAgentObj();

    virtual void onCreate() override;

    // IEventHandler
    virtual void onEvent(const IEvent *pEvent) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

public:
    static bool isFloatingLyrMode(CSkinWnd *pSkinWnd);
    static void getLyrDispStyleSettings(CSkinWnd *pSkinWnd, string &strLyrStyle);
    static void setLyrDispStyleSettings(CSkinWnd *pSkinWnd, cstr_t szLyrStyle);

protected:
    void createInfoTextCtrl();

    void changeLyricsDisplayStyle(cstr_t szLyrDispalyStyle, bool bRedraw = true);

    void loadToolbar();

protected:
    CUIObject                   *m_pToolbar;
    bool                        m_bEnableToolbar;
    class CMPSkinInfoTextCtrl   *m_pInfoTextCtrl;

    bool                        m_bFloatingLyr;
    string                      m_strLyrDisplayStyleDefault;
    string                      m_strLyrDisplayStyleCurrent;
    CLyricShowObj               *m_pLyricsShow;
    vector<string>              m_vProperties;
    bool                        m_bEnableStaticTextStyle;

};

#endif // !defined(MPlayerUI_LyricShowAgentObj_h)
