// SkinToolTip.cpp: implementation of the CSkinToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinToolTip.h"

void fillInToolInfo(TOOLINFO &ti, HWND hWnd, uint32_t nIDTool)
{
    memset(&ti, 0, sizeof(TOOLINFO));
    ti.cbSize = sizeof(TOOLINFO);

    if (nIDTool == 0)
    {
        ti.hwnd = ::getParent(hWnd);
        ti.uFlags = TTF_IDISHWND;
        ti.uId = (uint32_t)hWnd;
    }
    else
    {
        ti.hwnd = hWnd;
        ti.uFlags = 0;
        ti.uId = nIDTool;
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinToolTip::CSkinToolTip()
{
    m_hWnd = nullptr;
    m_hWndParent = nullptr;
}

CSkinToolTip::~CSkinToolTip()
{

}

bool CSkinToolTip::create(Window *pWndParent)
{
    m_hWnd = CreateWindowEx(nullptr, TOOLTIPS_CLASS, nullptr,
        WS_POPUP, // force WS_POPUP
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        pWndParent->getHandle(), nullptr, getAppInstance(), nullptr);

    m_hWndParent = pWndParent->getHandle();

    return (m_hWnd != nullptr);
}

bool CSkinToolTip::addTool(cstr_t szText, CRect *lpRectTool, uint32_t nIDTool)
{
    assert(::isWindow(m_hWnd));
    assert(szText != nullptr);
    // the toolrect and toolid must both be zero or both valid
    assert((lpRectTool != nullptr && nIDTool != 0) ||
           (lpRectTool == nullptr) && (nIDTool == 0));

    TOOLINFO    ti;
    fillInToolInfo(ti, m_hWndParent, nIDTool);
    if (lpRectTool != nullptr)
        memcpy(&ti.rect, lpRectTool, sizeof(CRect));
    ti.lpszText = (char *)szText;

    return (bool) ::sendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti);
}

void CSkinToolTip::delTool(uint32_t nIDTool)
{
    if (!::isWindow(m_hWnd))
        return;

    TOOLINFO ti;
    fillInToolInfo(ti, m_hWndParent, nIDTool);

    ::sendMessage(m_hWnd, TTM_DELTOOL, 0, (LPARAM)&ti);
}

void CSkinToolTip::relayEvent(LPMSG lpMsg)
{
    assert(::isWindow(m_hWnd));
    ::sendMessage(m_hWnd, TTM_RELAYEVENT, 0, (LPARAM)lpMsg);
}
