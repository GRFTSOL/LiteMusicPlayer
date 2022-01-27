#pragma once

#include "../Skin/SkinLinearContainer.h"

class CLyricShowTxtContainer : public CSkinLinearContainer  
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinLinearContainer)
public:
    CLyricShowTxtContainer();
    virtual ~CLyricShowTxtContainer();

    void onCreate();

    bool setProperty(cstr_t szProperty, cstr_t szValue);

protected:
    vector<string>            m_vProperties;
    CUIObject                *m_pLyricsShow;
    CUIObject                *m_pObjScrollBar;

};
