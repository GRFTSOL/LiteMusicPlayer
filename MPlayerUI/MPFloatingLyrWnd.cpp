#include "MPlayerApp.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowAgentObj.h"


CMPFloatingLyrWnd g_wndFloatingLyr;



CMPFloatingLyrWnd::CMPFloatingLyrWnd() {
    m_bManageBySkinFactory = false;
}

CMPFloatingLyrWnd::~CMPFloatingLyrWnd() {

}

int CMPFloatingLyrWnd::create() {
    m_rcBoundBox.left = g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_left", m_rcBoundBox.left);
    m_rcBoundBox.top = g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_top", m_rcBoundBox.top);
    m_rcBoundBox.right = m_rcBoundBox.left + g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_width", m_rcBoundBox.width());
    m_rcBoundBox.bottom = m_rcBoundBox.top + g_profile.getInt(SZ_SECT_FLOATING_LYR, "wnd_height", m_rcBoundBox.height());

    SkinWndStartupInfo skinWndStartupInfo("FloatingLyr", "Floating Lyrics",
        "floatinglyr.xml", nullptr);
    CMPSkinWnd::create(skinWndStartupInfo, CMPlayerAppBase::getMPSkinFactory());

    m_cursor.loadCursorFromRes(IDC_MLHAND);

    return ERR_OK;
}

bool CMPFloatingLyrWnd::onCustomCommand(int nId) {
    if (nId == CMD_CLOSE) {
        g_profile.writeInt("FloatingLyr", false);
    }

    return CMPSkinWnd::onCustomCommand(nId);
}

bool CMPFloatingLyrWnd::settingGetTopmost() {
    return g_profile.getBool(SZ_SECT_FLOATING_LYR, "topmost", true);
}

int CMPFloatingLyrWnd::settingGetOpaquePercent() {
    return g_profile.getInt(SZ_SECT_UI, "WindowOpaquePercent", 100);
}

bool CMPFloatingLyrWnd::settingGetClickThrough() {
    return g_profile.getBool(SZ_SECT_FLOATING_LYR, "ClickThrough", false);
}

void CMPFloatingLyrWnd::settingReverseTopmost() {
    bool bTopmost = !isTopmost();

    g_profile.writeInt(SZ_SECT_FLOATING_LYR, "topmost", bTopmost);
    setTopmost(bTopmost);
}

void CMPFloatingLyrWnd::settingSetOpaquePercent(int nPercent) {
    g_profile.writeInt(SZ_SECT_UI, "WindowOpaquePercent", nPercent);
    setTransparent(opaquePercentToAlpha(nPercent), m_bClickThrough);
}

void CMPFloatingLyrWnd::settingReverseClickThrough() {
    g_profile.writeInt(SZ_SECT_FLOATING_LYR, "ClickThrough", !m_bClickThrough);
    m_bClickThrough = !m_bClickThrough;
    setTransparent(opaquePercentToAlpha(settingGetOpaquePercent()),
        m_bClickThrough);
}
