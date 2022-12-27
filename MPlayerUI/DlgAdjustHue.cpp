// DlgAdjustHue.cpp: implementation of the CDlgAdjustHue class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "DlgAdjustHue.h"


class CPageAdjustHue : public CSkinContainer, public IScrollNotify
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    // IScrollNotify
    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override
    {
        updateAdjustHue((float)nPos);
    }

    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override
    {
        updateAdjustHue((float)nPos);
    }

    void updateAdjustHue(float hue)
    {
        g_profile.writeInt(CMPlayerAppBase::getMPSkinFactory()->getSkinName(), "Hue", (int)hue);
        CMPlayerAppBase::getMPSkinFactory()->adjustHue(hue);
    }

    void onCreate() override
    {
        CSkinContainer::onCreate();

        m_hueOld = (float)(g_profile.getInt(CMPlayerAppBase::getMPSkinFactory()->getSkinName(), "Hue", 0));

        CSkinSeekCtrl *pSeek = (CSkinSeekCtrl*)getUIObjectById("CID_HUE", CSkinSeekCtrl::className());
        if (pSeek)
        {
            pSeek->setScrollInfo(0, 360, 30, (int)m_hueOld, 1, false);
            pSeek->setScrollNotify(this);
        }
    }

    bool onCustomCommand(int nId) override
    {
        if (nId == getIDByName("CID_RESET"))
        {
            updateAdjustHue(0.0);
            CSkinSeekCtrl *pSeek = (CSkinSeekCtrl*)getUIObjectById(getIDByName("CID_HUE"), CSkinSeekCtrl::className());
            if (pSeek)
                pSeek->setScrollPos(0);
            return true;
        }

        return CSkinContainer::onCustomCommand(nId);
    }

    bool onCancel() override
    {
        updateAdjustHue(m_hueOld);
        return CSkinContainer::onCancel();
    }

protected:
    float                    m_hueOld;

};

UIOBJECT_CLASS_NAME_IMP(CPageAdjustHue, "Container.adjustHue")

//////////////////////////////////////////////////////////////////////

void showAdjustHueDialog(CSkinWnd *pParent)
{
    CSkinApp::getInstance()->showDialog(pParent, "adjustHue.xml");
}

void registerAdjustHuePage(CSkinFactory *pSkinFactory)
{
    AddUIObjNewer2(pSkinFactory, CPageAdjustHue);
}

