#pragma once

#ifndef _SKIN_TOOL_TOP_INC_
#define _SKIN_TOOL_TOP_INC_

#include "Window.h"


class CSkinToolTip {
public:
    CSkinToolTip();
    virtual ~CSkinToolTip();

    void delTool(uint32_t nIDTool);
    bool addTool(cstr_t szText, CRect *lpRectTool = nullptr, uint32_t nIDTool = 0);
    bool create(Window *pWndParent);

    bool isWindow() { return isValid(); }
    bool isValid() { return m_hWnd != nullptr; }

    void destroy();

protected:
    void                        *       m_hWnd;

};

#endif // !defined(Window_mac_SkinToolTip_h)
