#pragma once

#ifndef MPlayerUI_win32_HotkeyCtrlEx_h
#define MPlayerUI_win32_HotkeyCtrlEx_h


void formatHotkeyText(string &strText, uint32_t nVirtKey, uint32_t fsModifiers);

bool isKeyPressed(int nKey);

class CHotkeyCtrlEx : public Window {
public:
    CHotkeyCtrlEx();
    virtual ~CHotkeyCtrlEx();

    bool init(Window *pParentWnd, uint32_t uChildId);

    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags);
    virtual void onKeyUp(uint32_t nChar, uint32_t nFlags);

    void getHotkey(uint32_t &nVirKey, uint32_t &nfsModifiers);
    void setHotkey(uint32_t nVirKey, uint32_t nfsModifiers);

protected:
    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

    void updateHotKeyText();

    WNDPROC                     m_pWndProcOrg;

    bool                        m_bShiftDown, m_bAltDown, m_bCtrlDown, m_bWinDown;

    bool                        m_bCharDown;
    uint32_t                    m_nChar;

    uint32_t                    m_nHotVk, m_nfsModifiers;

};

#endif // !defined(MPlayerUI_win32_HotkeyCtrlEx_h)
