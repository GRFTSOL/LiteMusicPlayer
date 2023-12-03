#include "MPlayerApp.h"
#include "MPSkinFactory.h"
#include "MPSkinWnd.h"
#include "LyricShowMultiRowObj.h"
#include "LyricShowTxtObj.h"
#include "LyricShowTwoRowObj.h"
#include "LyricShowSingleRowObj.h"
#include "LyricShowVobSub.h"
#include "LyricShowAgentObj.h"
#include "LyricShowTextEditObj.h"
#include "LyricShowTxtContainer.h"
#include "SkinRateCtrl.h"
#include "SkinPicText.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"
#include "SkinFilterCtrl.h"
#include "MPSkinInfoTextCtrlEx.h"
#include "DlgAbout.h"
#include "DlgSearchLyrics.h"
#include "PreferenceDlg.h"
#include "DlgUpload.h"
#include "DlgAdjustHue.h"
#include "MPlaylistCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"

#ifdef _MPLAYER
#include "SkinTreeCtrl.h"
#endif

#ifdef _WIN32
#include "win32/LyricsDownloader.h"
#endif

#include "MPSkinMenu.h"

#include "MPFloatingLyrWnd.h"


CMPSkinFactory::CMPSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[])
: CSkinFactory(pApp, uidDefinition) {
    m_bClickThrough = false;
}

CMPSkinFactory::~CMPSkinFactory() {

}

int CMPSkinFactory::init() {
    AddUIObjNewer(CLyricShowMultiRowObj);
    AddUIObjNewer(CLyricShowTxtObj);
    AddUIObjNewer(CLyricShowTwoRowObj);
    AddUIObjNewer(CLyricShowSingleRowObj);
    AddUIObjNewer(CLyricShowVobSub);
    AddUIObjNewer(CLyricShowTextEditObj);
    AddUIObjNewer(CLyricShowTxtContainer);
    AddUIObjNewer(CLyricShowAgentObj);
    AddUIObjNewer(CSkinRateCtrl);
    AddUIObjNewer(CSkinPicText);
    AddUIObjNewer(CMPSkinTimeCtrl<CSkinPicText>);
    AddUIObjNewer(CMPSkinTimeCtrl<CSkinStaticText>);
    AddUIObjNewer(CMPSkinInfoTextCtrl);
    AddUIObjNewer(CMPSkinInfoTextCtrlEx);
    AddUIObjNewer(CSkinFilterCtrl);
#ifdef _MPLAYER
    AddUIObjNewer(CSkinTreeCtrl);
#endif
    AddUIObjNewer(CMPSkinMediaNumInfoCtrl);
    AddUIObjNewer(CMediaInfoTextCtrl);
    AddUIObjNewer(CMediaAlbumArtCtrl);
    AddUIObjNewer(CMPlaylistCtrl);

    registerAboutPage(this);
    registerSearchLyricsPage(this);
    registerPreferencePage(this);
    registerUploadLyrPage(this);
    registerAdjustHuePage(this);

    return CSkinFactory::init();
}

CSkinWnd *CMPSkinFactory::newSkinWnd(cstr_t szSkinWndName, bool bMainWnd) {
    if (bMainWnd) {
        return new CMPSkinMainWnd;
    } else {
        return new CMPSkinWnd;
    }
}

SkinMenuPtr CMPSkinFactory::newSkinMenu(CSkinWnd *pWnd, const rapidjson::Value &items) {
    auto menu = std::make_shared<CMPSkinMenu>();
    menu->addUICheckStatusIf(pWnd);
    menu->loadMenu(items);
    return menu;
}

void CMPSkinFactory::topmostAll(bool bTopmost) {
    auto itEnd = m_listSkinWnds.end();

    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;
        pWnd->setTopmost(bTopmost);
    }
}

void CMPSkinFactory::minizeAll(bool bSilently) {
    auto itEnd = m_listSkinWnds.end();

    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;
#ifdef _WIN32
        if (::getParent(pWnd->getHandle()) != nullptr) {
            continue;
        }
#endif
        if (pWnd->isToolWindow()) {
            pWnd->minimizeNoActivate();
            pWnd->hide();
        } else if (bSilently) {
            pWnd->minimizeNoActivate();
        } else {
            pWnd->minimize();
        }
    }
}

void CMPSkinFactory::restoreAll() {
    auto itEnd = m_listSkinWnds.end();

    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;

        if (pWnd->isIconic()) {
            pWnd->showNoActivate();
        }

        if (pWnd->isToolWindow()) {
            pWnd->show();
        }
    }
}

int CMPSkinFactory::getIDByNameEx(cstr_t szId, string &strToolTip) {
    int nID = CSkinFactory::getIDByNameEx(szId, strToolTip);
    if (nID == ID_INVALID) {
        return nID;
    }

    string strHotkey;

    if (CMPlayerAppBase::getHotkey().getHotkeyText(nID, strHotkey)) {
        strToolTip = _TL(strToolTip.c_str());
        strToolTip += " (";
        strToolTip += strHotkey;
        strToolTip += ")";
    }

    return nID;
}

string CMPSkinFactory::getTooltip(int nId) {
    auto toolTip = CSkinFactory::getTooltip(nId);

    if (toolTip.size()) {
        string strHotkey;

        if (CMPlayerAppBase::getHotkey().getHotkeyText(nId, strHotkey)) {
            toolTip = _TL(toolTip.c_str());
            toolTip += " (";
            toolTip += strHotkey;
            toolTip += ")";
        }
    }

    return toolTip;
}

void CMPSkinFactory::adjustHue(float hue, float saturation, float luminance) {
    CSkinFactory::adjustHue(hue, saturation, luminance);

    g_wndFloatingLyr.onAdjustHue(hue, saturation, luminance);
    g_wndFloatingLyr.invalidateRect();
}

#ifndef _MPLAYER
void CMPSkinFactory::beforeTrackMoveWith(Window *pWndChain[], int nCount, Window *pWndToTrack) {
    auto itEnd = m_listSkinWnds.end();

    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;
        pWnd->m_WndDrag.beforeTrackMoveWith(pWndChain, nCount, pWndToTrack);
    }
}

void CMPSkinFactory::trackMoveWith(Window *pWnd, int x, int y) {
    auto itEnd = m_listSkinWnds.end();

    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;
        pWnd->m_WndDrag.trackMoveWith(pWnd, x, y);
    }
}

void CMPSkinFactory::addWndCloseto(Window *pWnd, cstr_t szWndName, cstr_t szClass) {
    m_WndCloseToPlayers.addWndCloseto(pWnd, szClass, szWndName);
}

// ISkinWndDragHost
void CMPSkinFactory::getWndDragAutoCloseTo(vector<Window *> &vWnd) {
    CSkinFactory::getWndDragAutoCloseTo(vWnd);

    V_WNDCLOSETO::iterator it, itEnd;
    itEnd = m_WndCloseToPlayers.m_vWndCloseTo.end();
    for (it = m_WndCloseToPlayers.m_vWndCloseTo.begin(); it != itEnd; ++it) {
        Window *pWnd;
        WndDrag::WndCloseTo &item = *it;

        if (item.pWnd) {
            pWnd = item.pWnd;
        } else {
            const char *szClass = item.strClass.c_str();
            if (isEmptyString(szClass)) {
                szClass = nullptr;
            }
            const char *szWnd = item.strWndName.c_str();
            if (isEmptyString(szWnd)) {
                szWnd = nullptr;
            }
            pWnd = findWindow(szClass, szWnd);
        }

        if (pWnd) {
            vWnd.push_back(pWnd);
        }
    }
}
#endif

void CMPSkinFactory::allUpdateTransparent() {
    int nOpaque = g_profile.getInt(SZ_SECT_UI, "WindowOpaquePercent", 100);
    uint8_t byAlpha = opaquePercentToAlpha(nOpaque);

#ifdef _WIN32
    if (!isLayeredWndSupported()) {
        return;
    }

#ifndef _MPLAYER
    if (g_wndFloatingLyr.isValid()) {
        g_wndFloatingLyr.setTransparent(byAlpha, false);
    }

    if (::getParent(CMPlayerAppBase::getMainWnd()->getHandle())) {
        return;
    }
#endif
#endif // #ifdef _WIN32

    auto itEnd = m_listSkinWnds.end();
    for (auto it = m_listSkinWnds.begin(); it != itEnd; ++it) {
        CSkinWnd *pWnd = *it;
        pWnd->setTransparent(byAlpha, m_bClickThrough);
    }
}

void CMPSkinFactory::setClickThrough(bool bClickThrough) {
    m_bClickThrough = bClickThrough;

    // save ClickThrough options
    g_profile.writeInt(SZ_SECT_UI, "ClickThrough", m_bClickThrough);

#ifdef _WIN32
    if (m_bClickThrough) {
        // show MiniLyrics system tray icon.
        int nShowPos;

        nShowPos = g_profile.getInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);
        if (nShowPos == SHOW_ICON_ON_NONE || nShowPos == SHOW_ICON_ON_TASKBAR) {
            g_profile.writeInt(SZ_SECT_UI, "ShowIconOn", SHOW_ICON_ON_TASKBAR);
            CMPlayerAppBase::getMainWnd()->getTrayIcon().updateShowIconPos();
        }
    }
#endif // #ifdef _WIN32

    allUpdateTransparent();
}
