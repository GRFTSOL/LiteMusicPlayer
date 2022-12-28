#pragma once

#ifndef MPlayerUI_gtk2_HotkeyCtrlEx_h
#define MPlayerUI_gtk2_HotkeyCtrlEx_h


void formatHotkeyText(string &strText, uint32_t nVirtKey, uint32_t fsModifiers);

class CHotkeyCtrlEx : public CWidgetCtrlBase {
public:
    CHotkeyCtrlEx();
    virtual ~CHotkeyCtrlEx();

    bool init(Window *pParentWnd, uint32_t uChildId);

    virtual void onKeyDown(uint32_t nChar, uint32_t nFlags);
    virtual void onKeyUp(uint32_t nChar, uint32_t nFlags);

    void getHotkey(uint32_t &nVirKey, uint32_t &nfsModifiers);
    void setHotkey(uint32_t nVirKey, uint32_t nfsModifiers);

protected:
    void updateHotKeyText();

    bool                        m_bShiftDown, m_bAltDown, m_bCtrlDown, m_bWinDown;

    bool                        m_bCharDown;
    uint32_t                    m_nChar;

    uint32_t                    m_nHotVk, m_nfsModifiers;

};

#endif // !defined(MPlayerUI_gtk2_HotkeyCtrlEx_h)
