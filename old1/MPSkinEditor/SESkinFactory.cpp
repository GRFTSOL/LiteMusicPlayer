// SESkinFactory.cpp: implementation of the CSESkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinEditor.h"
#include "SESkinFactory.h"
#include "../MPShared/SkinTextButton.h"
#include "../MPShared/SkinStaticText.h"
#include "../MPShared/SkinSeekCtrl.h"
#include "../MPShared/SkinListCtrl.h"
#include "../MPShared/SkinRateCtrl.h"
#include "../MPlayer/MPSkinVis.h"
#include "../MPShared/SkinBarFrame.h"
#include "../MPShared/SkinBarFrameClientArea.h"
#include "../MPShared/SkinContainerBar.h"
#include "../MPShared/SkinToolbar.h"
#include "../MPShared/SkinTreeCtrl.h"
#include "MPSkinTimeCtrl.h"
#include "MPSkinMediaNumInfoCtrl.h"
#include "MPSkinInfoTextCtrl.h"
#include "MPPPCWorkArea.h"
#include "UIEditObject.h"
#include "SkinUIObjAny.h"
#include "SESkinWnd.h"
#include "SESkinContainer.h"
#include "SEUIEditObject.h"
#include "MediaInfoTextCtrl.h"
#include "MediaAlbumArtCtrl.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinFactory::CSESkinFactory()
{

}

CSESkinFactory::~CSESkinFactory()
{

}

void CSESkinFactory::quit()
{
    LIST_SKINWNDS::iterator    it, itEnd;
    string        strOpenedSkinWnds;

    itEnd = m_listSkinWnds.end();
    for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
    {
        CSkinWnd    *pSkin = *it;
        if (!strOpenedSkinWnds.empty())
            strOpenedSkinWnds += ",";
        strOpenedSkinWnds += pSkin->getSkinWndName();
    }

    g_profile.writeString("LatestOpenedSkinWnds", strOpenedSkinWnds.c_str());

    close();
}

CUIObject *CSESkinFactory::createUIObject(CSkinWnd *pSkin, cstr_t szClassName)
{
    CUIObject        *pObj = nullptr;
    CSESkinWnd        *pSESkinWnd;

    pSESkinWnd = (CSESkinWnd *)pSkin;
    ISESkinNotify    *pNotify = pSESkinWnd->getNotify();

    if (strcasecmp(szClassName, CSkinButton::className()) == 0)
        pObj = new CSEUIEditObject<CSkinButton>(pNotify);
    else if (strcasecmp(szClassName, CSkinActiveButton::className()) == 0 ||
        strcasecmp(szClassName, "FocusButton") == 0)
        pObj = new CSEUIEditObject<CSkinActiveButton>(pNotify);
    else if (strcasecmp(szClassName, CSkinImage::className()) == 0)
        pObj = new CSEUIEditObject<CSkinImage>(pNotify);
    else if (strcasecmp(szClassName, CSkinNStatusImage::className()) == 0)
        pObj = new CSEUIEditObject<CSkinNStatusImage>(pNotify);
    else if (strcasecmp(szClassName, CSkinActiveImage::className()) == 0|| 
        strcasecmp(szClassName, "FocusImage") == 0)
        pObj = new CSEUIEditObject<CSkinActiveImage>(pNotify);
    else if (strcasecmp(szClassName, CSkinXScaleImage::className()) == 0||
        strcasecmp(szClassName, "XScaleImage") == 0)
        pObj = new CSEUIEditObject<CSkinXScaleImage>(pNotify);
    else if (strcasecmp(szClassName, CSkinYScaleImage::className()) == 0 ||
        strcasecmp(szClassName, "YScaleImage") == 0)
        pObj = new CSEUIEditObject<CSkinYScaleImage>(pNotify);
    else if (strcasecmp(szClassName, CSkinVScrollBar::className()) == 0)
        pObj = new CSEUIEditObject<CSkinVScrollBar>(pNotify);
    else if (strcasecmp(szClassName, CSkinVScrollBarOSStyle::className()) == 0)
        pObj = new CSEUIEditObject<CSkinVScrollBarOSStyle>(pNotify);
    else if (strcasecmp(szClassName, CSkinHScrollBarOSStyle::className()) == 0)
        pObj = new CSEUIEditObject<CSkinHScrollBarOSStyle>(pNotify);
    else if (strcasecmp(szClassName, CSkinCaption::className()) == 0)
        pObj = new CSEUIEditObject<CSkinCaption>(pNotify);
    else if (strcasecmp(szClassName, CSkinNStatusButton::className()) == 0)
        pObj = new CSEUIEditObject<CSkinNStatusButton>(pNotify);
    else if (strcasecmp(szClassName, CSkinRateCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CSkinRateCtrl>(pNotify);
    else if (strcasecmp(szClassName, CSkinStaticText::className()) == 0)
        pObj = new CSEUIEditObject<CSkinStaticText>(pNotify);
    else if (strcasecmp(szClassName, CSkinToolbar::className()) == 0)
        pObj = new CSEUIEditObject<CSkinToolbar>(pNotify);
    else if (strcasecmp(szClassName, CSkinTreeCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CSkinTreeCtrl>(pNotify);
    else if (strcasecmp(szClassName, CSkinTextButton::className()) == 0)
        pObj = new CSEUIEditObject<CSkinTextButton>(pNotify);
    else if (strcasecmp(szClassName, CSkinSeekCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CSkinSeekCtrl>(pNotify);
    else if (strcasecmp(szClassName, CSkinListCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CSkinListCtrl>(pNotify);
    else if (strcasecmp(szClassName, CMPSkinVis::className()) == 0)
        pObj = new CSEUIEditObject<CMPSkinVis>(pNotify);
    else if (strcasecmp(szClassName, CMPSkinTimeCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CMPSkinTimeCtrl>(pNotify);
    else if (strcasecmp(szClassName, CMPSkinMediaNumInfoCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CMPSkinMediaNumInfoCtrl>(pNotify);
    else if (strcasecmp(szClassName, CMPSkinInfoTextCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CMPSkinInfoTextCtrl>(pNotify);
    else if (strcasecmp(szClassName, CMPPPCWorkArea::className()) == 0)
        pObj = new CSEUIEditObject<CMPPPCWorkArea>(pNotify);
    else if (strcasecmp(szClassName, CMediaInfoTextCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CMediaInfoTextCtrl>(pNotify);
    else if (strcasecmp(szClassName, CMediaAlbumArtCtrl::className()) == 0)
        pObj = new CSEUIEditObject<CMediaAlbumArtCtrl>(pNotify);
    // container
    else if (strcasecmp(szClassName, CSkinContainer::className()) == 0)
        pObj = new CSESkinContainer<CSkinContainer>(pNotify);
    else if (strcasecmp(szClassName, CSkinBarFrame::className()) == 0)
        pObj = new CSESkinContainer<CSkinBarFrame>(pNotify);
    else if (strcasecmp(szClassName, CSkinBarFrameClientArea::className()) == 0)
        pObj = new CSESkinContainer<CSkinBarFrameClientArea>(pNotify);
    else if (strcasecmp(szClassName, CSkinContainerBar::className()) == 0)
        pObj = new CSESkinContainer<CSkinContainerBar>(pNotify);
    else
    {
        // other ui object
        CSEUIEditObject<CSkinUIObjAny>        *pObjAny = new CSEUIEditObject<CSkinUIObjAny>(pNotify);

        pObjAny->setClassName(szClassName);
        pObj = pObjAny;
    }

    if (pObj)
        pObj->m_pSkin = pSkin;

    return pObj;
/*
    if (pObj == nullptr)
    {
    }

    if (!pObj)
        pObj = new CSkinUIObjAny(szClassName);

    pObj->m_pSkin = pSkin;

    pObjEdit = new CUIEditObject;
    pObjEdit->m_pSkin = pSkin;

    pObjEdit->setContainedUIObject(pObj);

    // set property changed notify
    CSESkinWnd    *pSESkinWnd;
    pSESkinWnd = (CSESkinWnd *)pSkin;
    pObjEdit->setNotify(pSESkinWnd->getNotify());

    return pObjEdit;*/
}
/*

CUIObject *CSESkinFactory::createUIObject(CSkinWnd *pSkin, cstr_t szClassName)
{
    CUIObject        *pObj;
    CUIEditObject    *pObjEdit = nullptr;

    if (strcasecmp(szClassName, CSkinContainer::className()) == 0)
    {
        pObj = new CSESkinContainer;

        pObj->m_pSkin = pSkin;

        // set property changed notify
        CSESkinWnd    *pSESkinWnd;
        pSESkinWnd = (CSESkinWnd *)pSkin;
        ((CSESkinContainer*)pObj)->setNotify(pSESkinWnd->getNotify());

        return pObj;
    }
    else
        pObj = CSkinFactory::createUIObject(pSkin, szClassName);

    if (pObj == nullptr)
    {
        if (strcasecmp(szClassName, CSkinRateCtrl::className()) == 0)
            pObj = new CSkinRateCtrl;
        else if (strcasecmp(szClassName, CSkinStaticText::className()) == 0)
            pObj = new CSkinStaticText;
        else if (strcasecmp(szClassName, CSkinTextButton::className()) == 0)
            pObj = new CSkinTextButton;
        else if (strcasecmp(szClassName, CSkinSeekCtrl::className()) == 0)
            pObj = new CSkinSeekCtrl;
        else if (strcasecmp(szClassName, CSkinListCtrl::className()) == 0)
            pObj = new CSkinListCtrl;
        else if (strcasecmp(szClassName, CMPSkinVis::className()) == 0)
            pObj = new CMPSkinVis;
        else if (strcasecmp(szClassName, CMPSkinTimeCtrl::className()) == 0)
            pObj = new CMPSkinTimeCtrl;
        else if (strcasecmp(szClassName, CMPSkinMediaNumInfoCtrl::className()) == 0)
            pObj = new CMPSkinMediaNumInfoCtrl;
        else if (strcasecmp(szClassName, CMPSkinInfoTextCtrl::className()) == 0)
            pObj = new CMPSkinInfoTextCtrl;
        else if (strcasecmp(szClassName, CMPPPCWorkArea::className()) == 0)
            pObj = new CMPPPCWorkArea;
        else if (strcasecmp(szClassName, CSkinBarFrame::className()) == 0)
            pObj = new CSkinBarFrame;
        else if (strcasecmp(szClassName, CSkinBarFrameClientArea::className()) == 0)
            pObj = new CSkinBarFrameClientArea;
        else if (strcasecmp(szClassName, CSkinContainerBar::className()) == 0)
            pObj = new CSkinContainerBar;
    }

    if (!pObj)
        pObj = new CSkinUIObjAny(szClassName);

    pObj->m_pSkin = pSkin;

    pObjEdit = new CUIEditObject;
    pObjEdit->m_pSkin = pSkin;

    pObjEdit->setContainedUIObject(pObj);

    // set property changed notify
    CSESkinWnd    *pSESkinWnd;
    pSESkinWnd = (CSESkinWnd *)pSkin;
    pObjEdit->setNotify(pSESkinWnd->getNotify());

    return pObjEdit;
}
*/

CSkinWnd *CSESkinFactory::newSkinWnd(cstr_t szSkinWndName, bool bMainWnd)
{
    return new CSESkinWnd();
}

bool CSESkinFactory::loadMenu(CMenu **ppMenu, cstr_t szMenu)
{
/*    if (strcasecmp(szMenu, "MainMenu") == 0)
    {
        *ppMenu = new CMainMenu();
        return (*ppMenu)->loadPopupMenu(IDM_MENU, 0);
    }
    else if (strcasecmp(szMenu, "LyricsMenu") == 0)
    {
        *ppMenu = new CMainMenu();
        return (*ppMenu)->loadPopupMenu(IDM_MENU, 1);
    }
    else if (strcasecmp(szMenu, "PlaylistMenu") == 0)
    {
        *ppMenu = new CMainMenu();
        return (*ppMenu)->loadPopupMenu(IDM_MENU, 2);
    }
    else*/
        return false;
}

void CSESkinFactory::onLanguageChanged()
{
    LIST_SKINWNDS::iterator    it, itEnd;
    itEnd = m_listSkinWnds.end();
    for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
    {
        CSkinWnd    *pWnd = *it;
        pWnd->onLanguageChanged();
    }
}
