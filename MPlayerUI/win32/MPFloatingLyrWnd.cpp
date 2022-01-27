// MPFloatingLyrWnd.cpp: implementation of the CMPFloatingLyrWnd class.
//
//////////////////////////////////////////////////////////////////////
#include "MPlayerApp.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowAgentObj.h"

CMPFloatingLyrWnd        g_wndFloatingLyr;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPFloatingLyrWnd::CMPFloatingLyrWnd()
{
    m_bManageBySkinFactory = false;
}

CMPFloatingLyrWnd::~CMPFloatingLyrWnd()
{

}

int CMPFloatingLyrWnd::create()
{
    m_rcReal.left = g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_left", m_rcReal.left);
    m_rcReal.top = g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_top", m_rcReal.top);
    m_rcReal.right = m_rcReal.left + g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_width", m_rcReal.width());
    m_rcReal.bottom = m_rcReal.top + g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_height", m_rcReal.height());

    SkinWndStartupInfo skinWndStartupInfo("FloatingLyr", "Floating Lyrics", 
        "floatinglyr.xml", nullptr);
    CMPSkinWnd::create(skinWndStartupInfo, CMPlayerAppBase::getMPSkinFactory());

    m_cursor.loadCursorFromRes(IDC_MLHAND);

    return ERR_OK;
}

bool CMPFloatingLyrWnd::onCustomCommand(int nId)
{
    if (nId == CMD_CLOSE)
        g_profile.writeInt("FloatingLyr", false);

    return CMPSkinWnd::onCustomCommand(nId);
}

LRESULT CMPFloatingLyrWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
        {
            g_profile.writeInt("FloatingLyr", false);
        }
        break;
    }

    return CMPSkinWnd::wndProc(message, wParam, lParam);
}

bool CMPFloatingLyrWnd::settingGetTopmost()
{
    return g_profile.getBool(SZ_SECT_FLOATING_LYR, "topmost", true);
}

int CMPFloatingLyrWnd::settingGetOpaquePercent()
{
    return g_profile.getInt(SZ_SECT_UI, "WindowOpaquePercent", 100);
}

bool CMPFloatingLyrWnd::settingGetClickThrough()
{
    return g_profile.getBool(SZ_SECT_FLOATING_LYR, "ClickThrough", false);
}

void CMPFloatingLyrWnd::settingReverseTopmost()
{
    bool        bTopmost = !isTopmost();

    g_profile.writeInt(SZ_SECT_FLOATING_LYR, "topmost", bTopmost);
    setTopmost(bTopmost);
}

void CMPFloatingLyrWnd::settingSetOpaquePercent(int nPercent)
{
    g_profile.writeInt(SZ_SECT_UI, "WindowOpaquePercent", nPercent);
    setTransparent(opaquePercentToAlpha(nPercent), m_bClickThrough);
}

void CMPFloatingLyrWnd::settingReverseClickThrough()
{
    g_profile.writeInt(SZ_SECT_FLOATING_LYR, "ClickThrough", !m_bClickThrough);
    m_bClickThrough = !m_bClickThrough;
    setTransparent(opaquePercentToAlpha(settingGetOpaquePercent()),
        m_bClickThrough);
}
