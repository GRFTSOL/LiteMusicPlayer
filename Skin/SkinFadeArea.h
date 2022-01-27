// SkinFadeArea.h: interface for the CSkinFadeArea class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINFADEAREA_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
#define AFX_SKINFADEAREA_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_

#include "UIObject.h"

//
// This control is used to fade in/out the area that it covered.
//
class CSkinFadeArea : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinFadeArea();
    virtual ~CSkinFadeArea();

    void draw(CRawGraph *canvas);

};

#endif // !defined(AFX_SKINFADEAREA_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
