#include "MPlayerApp.h"
#include "PreferPageUI.h"
#include "PreferenceDlg.h"


//////////////////////////////////////////////////////////////////////////
const int _CMD_ID_LANGUANG_BEGIN = 3000;

class CPagePfUI : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfUI() : CPagePfBase(PAGE_UI, "ID_UI") {
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        addOptBool(ET_NULL, SZ_SECT_UI, "topmost", true, "CID_C_ON_TOP");

        // Click though...
        setUIObjectProperty("CID_C_CLICK_THROUGH", "CurStatus",
            stringPrintf("%d", CMPlayerAppBase::getMPSkinFactory()->getClickThrough()).c_str());

        initCheckButtons();

        updateLanguageInfo();
    }

    virtual bool onCommand(uint32_t nId) override {
        //
        // 选择了一个语言包, 则立即切换到该语言
        //
        if (nId == getIDByName("CID_C_ON_TOP")) {
            bool bValue = isButtonChecked(nId);
            g_profile.writeInt(SZ_SECT_UI, "topmost", bValue);
            CMPlayerAppBase::getMPSkinFactory()->topmostAll(bValue);
        } else if (nId == getIDByName("CID_C_CLICK_THROUGH")) {
            CMPlayerAppBase::getMPSkinFactory()->setClickThrough(isButtonChecked(nId));
        } else if (nId == getIDByName("CID_SEL_LANGUAGE")) {
            //
            // 创建快捷菜单, 显示可以使用的语言包
            //
            int nCmd;
            CRect rc;

            m_menuLang.destroy();

            m_menuLang.createPopupMenu();

            nCmd = _CMD_ID_LANGUANG_BEGIN;
            m_vTransFiles.clear();

            CLanguageTool::listTransFiles(nullptr, m_vTransFiles);

            // search for full match
            for (CLanguageTool::V_TRANSFILEINFO::iterator it = m_vTransFiles.begin();
            it != m_vTransFiles.end(); it++)
                {
                CLanguageTool::TransFileInfo &item = *it;

                m_menuLang.appendItem(nCmd, item.strLanguage.c_str());
                nCmd++;
            }

            if (getUIObjectRect(getIDByName("CID_SEL_LANGUAGE"), rc)) {
                m_pSkin->clientToScreen(rc);
            }

            m_menuLang.trackPopupMenu(rc.left, rc.top, m_pSkin);
        } else if (nId >= _CMD_ID_LANGUANG_BEGIN &&
            nId < _CMD_ID_LANGUANG_BEGIN + (int)m_vTransFiles.size()) {
            string langFileName = g_profile.getString("Language", "");
            if (!langFileName.empty()) {
                if (m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName == langFileName) {
                    return true;
                }
            }

            g_profile.writeString("Language", m_vTransFiles[nId - _CMD_ID_LANGUANG_BEGIN].strFileName.c_str());

            CMPlayerAppBase::getInstance()->onLanguageChanged();

            updateLanguageInfo();

            invalidate();

            return true;
        } else {
            return CPagePfBase::onCommand(nId);
        }

        return true;
    }

protected:
    void updateLanguageInfo() {
        string strLangFile;
        string strInfo;

        if (CLanguageTool::getCurrentLanguageFile(strLangFile)) {
            CProfile profile;

            profile.init(strLangFile.c_str());
            profile.doCache();

            strInfo = _TLT("Translator:");
            strInfo += " ";
            strInfo += profile.getString("Info", "Translator", "");
            strInfo += "\r\n";
            strInfo += _TLT("Contact:");
            strInfo += " ";
            strInfo += profile.getString("Info", "Contact", "");
        }

        setUIObjectText("CID_LANGUAGE_INFO", strInfo.c_str(), true);
    }

protected:
    CMenu                       m_menuLang;

    // 每一个菜单项对应的文件
    CLanguageTool::V_TRANSFILEINFO  m_vTransFiles;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfUI, "PreferPage.UI")

//////////////////////////////////////////////////////////////////////////

class CPagePfSkin : public CPagePfBase {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfSkin() : CPagePfBase(PAGE_SKINS, "ID_SKINS") {
        m_pSkinList = nullptr;
        m_bIgnoreSkinListNotify = false;
        CID_SKIN_LIST = 0;
        TIMER_ID_SKIN_SEL_CHANGED = 0;
    }

    void onInitialUpdate() override {
        CPagePfBase::onInitialUpdate();

        m_bIgnoreSkinListNotify = true;

        CID_SKIN_LIST = getIDByName("CID_SKIN_LIST");
        m_pSkin->registerUIObjNotifyHandler(CID_SKIN_LIST, this);
        m_pSkinList = (CSkinListCtrl *)getUIObjectById(CID_SKIN_LIST, CSkinListCtrl::className());
        if (m_pSkinList) {
            m_pSkinList->addColumn("Skins", 200);
        }

        m_bIgnoreSkinListNotify = false;

        updateSkinList();
    }

    void onDestroy() override {
        m_pSkin->unregisterUIObjNotifyHandler(this);

        CPagePfBase::onDestroy();
    }

    void onTimer(int nId) override {
        if (nId != TIMER_ID_SKIN_SEL_CHANGED) {
            return;
        }

        m_pSkin->unregisterTimerObject(this, TIMER_ID_SKIN_SEL_CHANGED);

        if (!m_pSkinList) {
            return;
        }

        int nSel = m_pSkinList->getNextSelectedItem();
        if (nSel == -1) {
            return;
        }

        string strSkinName;
        m_pSkinList->getItemText(nSel, 0, strSkinName);

        CMPlayerAppBase::getInstance()->changeSkinByUserCmd(strSkinName.c_str());
    }

    virtual void onUIObjNotify(IUIObjNotify *pNotify) override {
        if (pNotify->nID == CID_SKIN_LIST && pNotify->isKindOf(CSkinListCtrl::className())) {
            if (m_bIgnoreSkinListNotify) {
                return;
            }

            CSkinListCtrlEventNotify *pListCtrlNotify = (CSkinListCtrlEventNotify*)pNotify;
            if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_SEL_CHANGED) {
                m_pSkin->unregisterTimerObject(this, TIMER_ID_SKIN_SEL_CHANGED);
                // 设置延时时间为300ms，即在300ms后，用户的选择才会生效
                TIMER_ID_SKIN_SEL_CHANGED = m_pSkin->registerTimerObject(this, 300);
            }
        }
    }

    void updateSkinList() {
        if (!m_pSkinList) {
            return;
        }

        m_bIgnoreSkinListNotify = true;

        m_pSkinList->deleteAllItems(false);

        string strDefaultSkin = CSkinApp::getInstance()->getDefaultSkin();

#ifdef _MINILYRICS_WIN32
        m_strEmbeddedSkinName = "";
        if (CMPlayerAppBase::getInstance()->isSupportEmbedded()) {
            m_strEmbeddedSkinName = string("<< ") + g_player.getEmbeddedSkinName() + " >>";
            m_pSkinList->insertItem(m_pSkinList->getItemCount(), m_strEmbeddedSkinName.c_str());

            if (CMPlayerAppBase::getInstance()->isEmbeddedMode()) {
                m_pSkinList->setItemSelectionState(0, true);
            }
        }
#endif

        // 查找所有的Skin，并且添加到菜单中
        vector<string> vSkins;
        if (CMPlayerAppBase::getMPSkinFactory()->enumAllSkins(vSkins)) {
            for (uint32_t i = 0; i < vSkins.size(); i++) {
                m_pSkinList->insertItem((int)m_pSkinList->getItemCount(), vSkins[i].c_str());
                if (strcmp(strDefaultSkin.c_str(), vSkins[i].c_str()) == 0) {
                    m_pSkinList->setItemSelectionState((int)m_pSkinList->getItemCount() - 1, true);
                }
            }
            updateSkinComment(strDefaultSkin.c_str());
        }

        m_bIgnoreSkinListNotify = false;
    }

    bool updateSkinComment(cstr_t szSkin) {
        string fnReadme;
        string str;

        // 设置Skin的说明
        if (CMPlayerAppBase::getMPSkinFactory()->getResourceMgr()->getResourcePathName("Readme.txt", fnReadme)
            && readFileByBom(fnReadme.c_str(), str)) {
            setUIObjectText("CID_SKIN_INFO", str.c_str(), true);
            return true;
        }

        setUIObjectText("CID_SKIN_INFO", "", true);
        return false;
    }

protected:
    int                         CID_SKIN_LIST;
    CSkinListCtrl               *m_pSkinList;
    string                      m_strEmbeddedSkinName;
    bool                        m_bIgnoreSkinListNotify;
    int                         TIMER_ID_SKIN_SEL_CHANGED;

};

UIOBJECT_CLASS_NAME_IMP(CPagePfSkin, "PreferPage.Skin")

//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CPagePfUIRoot, "PreferPage.UIRoot")

CPagePfUIRoot::CPagePfUIRoot() : CPagePfBase(PAGE_UNKNOWN, "ID_ROOT_UI") {
}

void CPagePfUIRoot::onInitialUpdate() {
    CPagePfBase::onInitialUpdate();

    checkToolbarDefaultPage("CID_TOOLBAR_UI");
}

void registerPfUIPages(CSkinFactory *pSkinFactory) {
    AddUIObjNewer2(pSkinFactory, CPagePfUIRoot);
    AddUIObjNewer2(pSkinFactory, CPagePfUI);
    AddUIObjNewer2(pSkinFactory, CPagePfSkin);
}
