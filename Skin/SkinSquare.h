// SkinSquare.h: interface for the CSkinSquare class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_)
#define AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"

class CSkinSquare : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinSquare();
    virtual ~CSkinSquare();

    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    CColor                m_clrBg;

};

#endif // !defined(AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_)
