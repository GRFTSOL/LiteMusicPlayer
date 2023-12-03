#include "HotkeyCtrlEx.h"


bool isExtendedKey(uint32_t nVk) {
    return nVk >= VK_SPACE && nVk <= VK_HELP;
}

void formatHotkeyText(string &strText, uint32_t nVirtKey, uint32_t fsModifiers) {
    //     char    szKeyName[256];
    //     uint32_t    nScanCode;
    //
    //     strText.resize(0);
    //
    //     emptyStr(szKeyName);
    //     nScanCode = MapVirtualKey(nVirtKey, 0) << 16;
    //     if (isExtendedKey(nVirtKey))
    //         nScanCode |= 1 << 24;
    //     GetKeyNameText(nScanCode, szKeyName, CountOf(szKeyName));
    //
    //     if (isFlagSet(fsModifiers, MOD_WIN))
    //         strText += "Winkey + ";
    //     if (isFlagSet(fsModifiers, MOD_CONTROL))
    //         strText += "Ctrl + ";
    //     if (isFlagSet(fsModifiers, MOD_SHIFT))
    //         strText += "Shift + ";
    //     if (isFlagSet(fsModifiers, MOD_ALT))
    //         strText += "Alt + ";
    //     strText += szKeyName;
}



CHotkeyCtrlEx::CHotkeyCtrlEx() {
}

CHotkeyCtrlEx::~CHotkeyCtrlEx() {

}

bool CHotkeyCtrlEx::init(Window *pParentWnd, uint32_t uChildId) {
    m_bShiftDown = false;
    m_bAltDown = false;
    m_bCtrlDown = false;
    m_bWinDown = false;
    m_bCharDown = false;

    m_nChar = 0;

    m_nHotVk = 0;
    m_nfsModifiers = 0;

    return true;
}

void CHotkeyCtrlEx::onKeyDown(uint32_t nChar, uint32_t nFlags) {

    updateHotKeyText();
}

void CHotkeyCtrlEx::onKeyUp(uint32_t nChar, uint32_t nFlags) {

    if (m_bCharDown) {
        updateHotKeyText();
    }
}

void CHotkeyCtrlEx::getHotkey(uint32_t &nVirKey, uint32_t &nfsModifiers) {
    nVirKey = m_nHotVk;
    nfsModifiers = m_nfsModifiers;
}

void CHotkeyCtrlEx::setHotkey(uint32_t nVirKey, uint32_t nfsModifiers) {
    m_nHotVk = nVirKey;
    m_nfsModifiers = nfsModifiers;

    string strText;

    formatHotkeyText(strText, m_nHotVk, m_nfsModifiers);

    setWindowText(strText.c_str());
}

void CHotkeyCtrlEx::updateHotKeyText() {
    //     if (m_bCharDown)
    //         m_nHotVk = m_nChar;
    //     else
    //         m_nHotVk = 0;
    //
    //     m_nfsModifiers = 0;
    //     if (m_bWinDown)
    //         m_nfsModifiers |= MOD_WIN;
    //     if (m_bShiftDown)
    //         m_nfsModifiers |= MOD_SHIFT;
    //     if (m_bCtrlDown)
    //         m_nfsModifiers |= MOD_CONTROL;
    //     if (m_bAltDown)
    //         m_nfsModifiers |= MOD_ALT;
    //
    //     string    strText;
    //
    //     formatHotkeyText(strText, m_nHotVk, m_nfsModifiers);
    //
    //     setWindowText(strText.c_str());
}
