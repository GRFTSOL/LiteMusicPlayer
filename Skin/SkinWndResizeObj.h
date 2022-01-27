#pragma once

#include "UIObject.h"

class CSkinWndResizeObj :
    public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinWndResizeObj(void);
    ~CSkinWndResizeObj(void);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void onSize();

protected:
    uint32_t                m_nResizeDirection;

};
