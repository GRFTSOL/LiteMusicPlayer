/********************************************************************
    Created  :    2001年12月6日 21:52:18
    FileName :    PreferenceDlg.cpp
    Author   :    xhy
    
    Purpose  :    
*********************************************************************/

#include "MPlayerApp.h"
#include "PreferenceDlg.h"
#include "PreferPageSystem.h"
#include "PreferPageUI.h"
#include "PreferPageLyrics.h"
#include "PreferPageTheme.h"
#include "PreferPageAdvanced.h"

#ifdef _MPLAYER
#include "PreferPageAssociation.h"
#endif

class CPagePfRoot : public CPagePfBase
{
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfRoot();

    void onInitialUpdate();

};

UIOBJECT_CLASS_NAME_IMP(CPagePfRoot, "PreferPage.Root")

CPagePfRoot::CPagePfRoot() : CPagePfBase(PAGE_UNKNOWN, "CMD_ROOT_UI")
{
}

void CPagePfRoot::onInitialUpdate()
{
    CPagePfBase::onInitialUpdate();

    setDefaultPreferPage(this, (PreferPageID)getExPoolInt(SZ_EX_POOL_PF_DEFAULT_PAGE));
    checkToolbarDefaultPage("CID_TB_PREFERENCE");
}

void showPreferenceDialog(CSkinWnd *pParent, bool bFloatingLyr, PreferPageID preferPageId)
{
    assert(preferPageId != PAGE_UNKNOWN);
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
        "Preferences.xml", pParent);

    skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_PF_FLOATING_LYR] = BOOLTOSTR(bFloatingLyr);
    skinWndStartupInfo.mapExchangePool[SZ_EX_POOL_PF_DEFAULT_PAGE] = CStrPrintf("%d", preferPageId).c_str();

    CSkinApp::getInstance()->getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}

void registerPreferencePage(CSkinFactory *pSkinFactory)
{
    registerPfUIPages(pSkinFactory);
    registerPfThemePages(pSkinFactory);
    registerPfSystemPages(pSkinFactory);
    registerPfLyricsPages(pSkinFactory);
    AddUIObjNewer2(pSkinFactory, CPagePfAdvanced);
    AddUIObjNewer2(pSkinFactory, CPagePfRoot);
}
