#pragma once

#ifndef Skin_SkinCaption_h
#define Skin_SkinCaption_h

#include "UIObject.h"


class CSkinCaption : public CUIObject {
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinCaption();
    virtual ~CSkinCaption();

    void draw(CRawGraph *canvas) override;

    bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    bool onLButtonDblClk(uint32_t nFlags, CPoint point) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    CSFImage                    m_imgBk, m_imgFocus;

    string                      m_strBmpBkFile;
    string                      m_strBmpFocusFile;

    bool                        m_bEnableMaximize;

    int                         m_nCutPos1, m_nCutPos2, m_nCutPos3, m_nCutPos4;

    BlendPixMode                m_bpm;

};

#endif // !defined(Skin_SkinCaption_h)
