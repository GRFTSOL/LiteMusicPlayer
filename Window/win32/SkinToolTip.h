// ToolTipWnd.h: interface for the CToolTipWnd class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SKIN_TOOL_TOP_INC_
#define _SKIN_TOOL_TOP_INC_

#include "BaseWnd.h"

class CSkinToolTip
{
public:
    CSkinToolTip();
    virtual ~CSkinToolTip();

    void relayEvent(LPMSG lpMsg);
    void delTool(uint32_t nIDTool);
    bool addTool(cstr_t szText = LPSTR_TEXTCALLBACK, CRect *lpRectTool = nullptr, uint32_t nIDTool = 0);
    bool create(Window *pWndParent);

    bool isValid() { return m_hWnd != nullptr; }
    bool isWindow() { return tobool(::isWindow(m_hWnd)); }

    void destroy() { ::destroyWindow(m_hWnd); m_hWnd = nullptr; }

protected:
    HWND        m_hWndParent;
    HWND        m_hWnd;

};

#endif // !defined(AFX_TOOLTIPWND_H__4D602F6E_4E58_4921_ACDE_3E8DBA83DBBE__INCLUDED_)
