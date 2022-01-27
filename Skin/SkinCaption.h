// SkinCaption.h: interface for the CSkinCaption class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
#define AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_

#include "UIObject.h"

class CSkinCaption : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinCaption();
    virtual ~CSkinCaption();

    void draw(CRawGraph *canvas);

    bool onLButtonDown(uint32_t nFlags, CPoint point);
    bool onLButtonUp(uint32_t nFlags, CPoint point);
    bool onLButtonDblClk(uint32_t nFlags, CPoint point);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    CSFImage        m_imgBk, m_imgFocus;

    string          m_strBmpBkFile;
    string          m_strBmpFocusFile;

    bool            m_bEnableMaximize;

    int             m_nCutPos1, m_nCutPos2, m_nCutPos3, m_nCutPos4;

    BlendPixMode    m_bpm;

};

#endif // !defined(AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
