#pragma once

#include "PopupSkinListWnd.h"
#include "SkinTextButton.h"

class CSkinComboBoxEventNotify : public IUIObjNotify
{
public:
    CSkinComboBoxEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_SEL_CHANGED; }
    enum Command
    {
        C_SEL_CHANGED,
    };

    Command                cmd;

};

class CSkinComboBox : public CSkinTextButton, public IPopupSkinWndNotify
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinTextButton);
public:
    CSkinComboBox();
    virtual ~CSkinComboBox();

    void onCreate();

    bool setProperty(cstr_t szProperty, cstr_t szValue);

protected:
    virtual void buttonDownAction();
    virtual void buttonUpAction();

public:
    virtual void popupSkinWndOnSelected();
    virtual void popupSkinWndOnDestroy();

    CPopupSkinListWnd &getListData() { return m_popupListWnd; }

public:
    // Interface for combo box.
    int getCount() const;

    virtual int getCurSel();
    void setCurSel(int nIndex);

    int addString(cstr_t szStr);
    int insertString(int nIndex, cstr_t lpszString);
    void setItemData(int nIndex, uint32_t nItemData);
    uint32_t getItemData(int nIndex);
    uint32_t getItemText(int nIndex, string &str);

    bool deleteString(uint32_t nIndex);

    void resetContent();

    bool getSelItemText(string &str);

protected:
    CPopupSkinListWnd        m_popupListWnd;

};
