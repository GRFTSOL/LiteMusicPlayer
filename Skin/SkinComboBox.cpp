#include "SkinTypes.h"
#include "Skin.h"
#include "SkinComboBox.h"


UIOBJECT_CLASS_NAME_IMP(CSkinComboBox, "ComboBox")

CSkinComboBox::CSkinComboBox() {
}

CSkinComboBox::~CSkinComboBox() {
}

void CSkinComboBox::onCreate() {
    CSkinTextButton::onCreate();

    m_popupListWnd.addColumn("", 100);
}

bool CSkinComboBox::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinTextButton::setProperty(szProperty, szValue)) {
        return true;
    }

    return true;
}

void CSkinComboBox::buttonDownAction() {
    CSkinTextButton::buttonDownAction();

    m_pSkin->releaseCaptureMouse(this);

    CRect rcPopup = m_rcObj;

    rcPopup.deflate(m_rcPadding.left, m_rcPadding.right);

    m_pSkin->clientToScreen(rcPopup);

    // popup list window
    m_popupListWnd.create(m_pSkin->getSkinFactory(), "ComboBoxPopup.xml",
        m_pSkin, this, rcPopup);
}

void CSkinComboBox::buttonUpAction() {
}

void CSkinComboBox::popupSkinWndOnSelected() {
    string str;
    if (getSelItemText(str)) {
        setProperty(SZ_PN_TEXT, str.c_str());
        invalidate();

        CSkinComboBoxEventNotify event(this);

        event.cmd = CSkinComboBoxEventNotify::C_SEL_CHANGED;
        m_pSkin->dispatchUIObjNotify(&event);
    }
}

void CSkinComboBox::popupSkinWndOnDestroy() {

}

// Interface for combo box.
int CSkinComboBox::getCount() const {
    return m_popupListWnd.getItemCount();
}

int CSkinComboBox::getCurSel() {
    return m_popupListWnd.getCurSel();
}

void CSkinComboBox::setCurSel(int nIndex) {
    m_popupListWnd.setCurSel(nIndex);

    string str;
    if (getSelItemText(str)) {
        setProperty(SZ_PN_TEXT, str.c_str());
    }
}

int CSkinComboBox::addString(cstr_t szStr) {
    return m_popupListWnd.insertItem(m_popupListWnd.getItemCount(), szStr);
}

int CSkinComboBox::insertString(int nIndex, cstr_t lpszString) {
    return m_popupListWnd.insertItem(nIndex, lpszString);
}

void CSkinComboBox::setItemData(int nIndex, uint32_t nItemData) {
    m_popupListWnd.setItemData(nIndex, nItemData);
}

uint32_t CSkinComboBox::getItemData(int nIndex) {
    return m_popupListWnd.getItemData(nIndex);
}

uint32_t CSkinComboBox::getItemText(int nIndex, string &str) {
    str = m_popupListWnd.getItemText(nIndex, 0);
    return (uint32_t)str.size();
}

bool CSkinComboBox::deleteString(uint32_t nIndex) {
    return m_popupListWnd.deleteItem(nIndex);
}

void CSkinComboBox::resetContent() {
    m_popupListWnd.deleteAllItems();

    setProperty(SZ_PN_TEXT, "");
}

bool CSkinComboBox::getSelItemText(string &str) {
    int n = m_popupListWnd.getCurSel();
    if (n == -1) {
        return false;
    }

    str = m_popupListWnd.getItemText(n, 0);
    return true;
}
