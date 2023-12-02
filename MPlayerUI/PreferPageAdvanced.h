#ifndef _PREFER_PAGE_ADVANCED_H_
#define _PREFER_PAGE_ADVANCED_H_

#pragma once

#include "PreferencePageBase.h"


class CPreferItem {
public:
    CPreferItem(cstr_t szName) : m_strName(szName) { }
    virtual ~CPreferItem() { }
    virtual string getValue() = 0;
    virtual void getOptions(VecStrings &vString, int &nRadio) = 0;
    virtual void setOption(int nIndex) = 0;
    virtual bool isDefault() = 0;
    virtual void reset() = 0;

    string                      m_strName;

};

typedef vector<CPreferItem *> VecPreferItems;


class CPagePfAdvanced : public CPagePfBase, public IEditNotification {
    UIOBJECT_CLASS_NAME_DECLARE(CPagePfBase)
public:
    CPagePfAdvanced();
    ~CPagePfAdvanced();

    void onInitialUpdate() override;

    void onDestroy() override;

    virtual void onUIObjNotify(IUIObjNotify *pNotify) override;

    bool onCommand(uint32_t nId) override;

    void onTimer(int nId) override;

    void onEditorTextChanged() override;

    void updateValues();

protected:
    enum {
        IMG_UNMODIFIED              = 0,
        IMG_MODIFIED,
    };

    CPreferItem *getCurItem();
    void showCustomizeMenu(int x, int y);

    void listAllItems();

protected:
    VecPreferItems              m_vPreferItems;
    VecPreferItems              m_vFilteredItems;
    int                         m_nMenuIdStart, m_nMenuIdEnd;
    CSkinListCtrl               *m_pListItems;
    string                      m_strLastSearch;

    int                         CID_LIST_SETTINGS, CID_E_ADV_SEARCH;
    bool                        m_bIgnoreSettingListNotify;
    int                         m_nTimerIdSearch;

};


#endif // _PREFER_PAGE_ADVANCED_H_
