// PreferencePageBase.cpp: implementation of the CPreferencePageBase class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerAppBase.h"
#include "PreferencePageBase.h"

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfBase, "PagePfBase")

CPagePfBase::CPagePfBase(PreferPageID pfPageId, cstr_t szAssociateTabButtonId)
{
    m_pfPageId = pfPageId;
    m_nAssociateTabButtonId = CSkinApp::getInstance()->getSkinFactory()->getIDByName(szAssociateTabButtonId);
    m_nDefaultPageAssociateTabButtonId = UID_INVALID;
}

CSkinContainer *getChildPreferPageParent(CSkinContainer *pContainer)
{
    for (int i = 0; i < pContainer->getChildrenCount(); i++)
    {
        CUIObject *p = pContainer->getChildByIndex(i);
        if (p->isContainer())
        {
            if (p->isKindOf(CPagePfBase::className()))
                return pContainer;
            else
            {
                CSkinContainer *pParent = getChildPreferPageParent(p->getContainerIf());
                if (pParent)
                    return pParent;
            }
        }
    }

    return nullptr;
}

void CPagePfBase::onInitialUpdate()
{
    CSkinContainer::onInitialUpdate();

    // Retrieve the default associate tab button id
    CSkinContainer *pPfPageParent = getChildPreferPageParent(this);
    if (pPfPageParent)
    {
        for (int i = 0; i < pPfPageParent->getChildrenCount(); i++)
        {
            CUIObject *p = pPfPageParent->getChildByIndex(i);
            if (p->isContainer() && p->isKindOf(CPagePfBase::className()))
            {
                CPagePfBase *pPfPage = (CPagePfBase *)p;
                m_nDefaultPageAssociateTabButtonId = pPfPage->getAssociateTabButtonId();
                if (!pPfPageParent->hasActivePage())
                    pPfPageParent->switchToPage(p->getClassName(), false, 0, false);
                break;
            }
        }
    }
}

void CPagePfBase::onDestroy()
{
    m_pSkin->unregisterUIObjNotifyHandler(this);

    clearOptions();

    CSkinContainer::onDestroy();
}

bool CPagePfBase::onSwitchToPageCmd(int nId)
{
    CSkinContainer *pPfPageParent = getChildPreferPageParent(this);
    if (pPfPageParent)
    {
        for (int i = 0; i < pPfPageParent->getChildrenCount(); i++)
        {
            CUIObject *p = pPfPageParent->getChildByIndex(i);
            if (p->isContainer() && p->isKindOf(CPagePfBase::className()))
            {
                CPagePfBase *pPfPage = (CPagePfBase *)p;
                if (pPfPage->getAssociateTabButtonId() == nId)
                {
                    pPfPageParent->switchToPage(pPfPage->getClassName(), false, 0, true);
                    if (pPfPage->getPfPageID() != PAGE_UNKNOWN)
                    {
                        // write default page in exchange pool data
                        setExPool(SZ_EX_POOL_PF_DEFAULT_PAGE, pPfPage->getPfPageID());
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

bool CPagePfBase::onCustomCommand(int nId)
{
    if (CSkinContainer::onCustomCommand(nId))
        return true;

    // Toolbar(Tab) check command
    if (onSwitchToPageCmd(nId))
        return true;

    {
        // Bool
        VOptBool::iterator    it, itEnd;
        string        strValue;
        itEnd = m_vOptBool.end();
        for (it = m_vOptBool.begin(); it != itEnd; ++it)
        {
            OptBool        &opt = *it;
            if (opt.nCtrl == nId)
            {
                bool    bValue;

                CSkinNStatusButton *pButton = (CSkinNStatusButton*)getUIObjectById(opt.nCtrl, CSkinNStatusButton::className());
                if (pButton)
                {
                    bValue = tobool(pButton->getStatus());
                    CMPlayerSettings::setSettings(opt.eventType, opt.strSection.c_str(),
                        opt.strSettingName.c_str(), bValue, opt.eventType != ET_INVALID);
                    return true;
                }
            }
        }
    }

    {
        //
        // process Radio int Buttons Command
        //
        VOptRadioInt::iterator    it, itEnd;
        itEnd = m_vOptRadioInt.end();
        for (it = m_vOptRadioInt.begin(); it != itEnd; ++it)
        {
            OptRadioInt        &opt = *it;

            for (int i = 0; i < (int)opt.vCtrls.size(); i++)
            {
                if (opt.vCtrls[i] == nId)
                {
                    CMPlayerSettings::setSettings(opt.eventType, opt.strSection.c_str(),
                        opt.strSettingName.c_str(), opt.vValues[i], opt.eventType != ET_INVALID);

                    // Uncheck all buttons, in this group.
                    for (int k = 0; k < (int)opt.vCtrls.size(); k++)
                    {
                        if (k != i)
                            checkButton(opt.vCtrls[k], false);
                    }
                    return true;
                }
            }
        }
    }

    return false;
}

void CPagePfBase::addOptBool(EventType evtType, cstr_t szSection, cstr_t szSettingName, bool bDefValue, cstr_t szCtrlId)
{
    OptBool    opt;
    opt.eventType = evtType;
    opt.strSection = szSection;
    opt.strSettingName = szSettingName;
    opt.nDefaultValue = bDefValue;
    opt.nCtrl = getIDByName(szCtrlId);

    m_vOptBool.push_back(opt);
}

void CPagePfBase::initCheckButtons()
{
    {
        //
        // Check Combo Str
        //
        VOptComboStr::iterator    it, itEnd;
        string        strValue;
        itEnd = m_vOptComboStr.end();
        for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
        {
            OptComboStr        &opt = *it;
            CSkinComboBox    *pCombBox = (CSkinComboBox *)m_pSkin->getUIObjectById(opt.nIDWidgetCombo, CSkinComboBox::className());
            if (!pCombBox)
                continue;

            m_pSkin->registerUIObjNotifyHandler(opt.nIDWidgetCombo, this);

            strValue = g_profile.getString(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.strDefaultValue.c_str());
            if (strValue.empty())
                strValue = opt.strDefaultValue;
            else
                opt.strDefaultValue = strValue;
            for (int i = 0; i < (int)opt.vValues.size(); i++)
            {
                if (strcasecmp(opt.vValues[i].c_str(), strValue.c_str()) == 0)
                {
                    pCombBox->setCurSel(i);
                    break;
                }
            }
        }
    }

    {
        //
        // init Check Bool Buttons
        //
        VOptBool::iterator    it, itEnd;
        itEnd = m_vOptBool.end();
        for (it = m_vOptBool.begin(); it != itEnd; ++it)
        {
            OptBool        &opt = *it;
            bool        bValue;

            bValue = g_profile.getBool(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.nDefaultValue);
            CSkinNStatusButton *pButton = (CSkinNStatusButton*)getUIObjectById(opt.nCtrl, CSkinNStatusButton::className());
            if (pButton)
                pButton->setStatus(bValue);
            else
                ERR_LOG1("Control: %s is NOT defined in skin.", m_pSkin->getSkinFactory()->getStringOfID(opt.nCtrl).c_str());
        }
    }

    {
        //
        // Check Radio Int Buttons
        //
        VOptRadioInt::iterator    it, itEnd;
        int        value;
        itEnd = m_vOptRadioInt.end();
        for (it = m_vOptRadioInt.begin(); it != itEnd; ++it)
        {
            OptRadioInt        &opt = *it;

            value = g_profile.getInt(opt.strSection.c_str(), opt.strSettingName.c_str(), opt.nDefaultValue);
            for (int i = 0; i < (int)opt.vValues.size(); i++)
            {
                if (opt.vValues[i] == value)
                {
                    checkButton(opt.vCtrls[i], true);
                    break;
                }
            }
        }
    }
}

void CPagePfBase::onUIObjNotify(IUIObjNotify *pNotify)
{
    if (pNotify->isKindOf(CSkinComboBox::className()))
    {
        //
        // process Combo Str select changed command
        //
        VOptComboStr::iterator    it, itEnd;
        string        strValue;
        itEnd = m_vOptComboStr.end();
        for (it = m_vOptComboStr.begin(); it != itEnd; ++it)
        {
            OptComboStr        &opt = *it;
            if (opt.nIDWidgetCombo != pNotify->nID
                || ((CSkinComboBoxEventNotify *)pNotify)->cmd != CSkinComboBoxEventNotify::C_SEL_CHANGED)
                continue;

            CSkinComboBox    *pCombbox = (CSkinComboBox *)pNotify->pUIObject;
            int                nSel;
            nSel = pCombbox->getCurSel();

            if (nSel >= 0 && nSel < (int)opt.vValues.size())
            {
                if (strcasecmp(opt.vValues[nSel].c_str(), opt.strDefaultValue.c_str()) != 0)
                {
                    // Value changed...
                    opt.strDefaultValue = opt.vValues[nSel];
                    CMPlayerSettings::setSettings(opt.eventType, opt.strSection.c_str(),
                        opt.strSettingName.c_str(), opt.vValues[nSel].c_str(), opt.eventType != ET_INVALID);
                }
            }
            return;
        }
    }
}

void CPagePfBase::clearOptions()
{
    m_vOptBool.clear();
    m_vOptComboStr.clear();
    m_vOptRadioInt.clear();
}

bool CPagePfBase::setDefaultPreferPage(CSkinContainer *pContainer, PreferPageID pfPageIdDefault)
{
    if (m_pfPageId == pfPageIdDefault)
        return true;

    for (int i = 0; i < pContainer->getChildrenCount(); i++)
    {
        CUIObject *p = pContainer->getChildByIndex(i);
        if (p->isContainer())
        {
            if (p->isKindOf(CPagePfBase::className()))
            {
                CPagePfBase    *pPageChild = (CPagePfBase *)p;
                if (pPageChild->setDefaultPreferPage(pPageChild, pfPageIdDefault))
                {
                    pPageChild->getParent()->switchToPage(pPageChild->getClassName(), false, 0, false);
                    m_nDefaultPageAssociateTabButtonId = pPageChild->getAssociateTabButtonId();
                    return true;
                }
            }
            else if (setDefaultPreferPage(p->getContainerIf(), pfPageIdDefault))
                return true;
        }
    }

    return false;
}

void CPagePfBase::checkToolbarDefaultPage(cstr_t szToolbarId)
{
    if (m_nDefaultPageAssociateTabButtonId != UID_INVALID)
        checkToolbarButton(getIDByName(szToolbarId), m_nDefaultPageAssociateTabButtonId, true);
}
