// HotkeyCtrlEx.h: interface for the CHotkeyCtrlEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_)
#define AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void formatHotkeyText(string &strText, uint32_t nVirtKey, uint32_t fsModifiers);

class CHotkeyCtrlEx : public CWidgetCtrlBase
{
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

    bool            m_bShiftDown, m_bAltDown, m_bCtrlDown, m_bWinDown;

    bool            m_bCharDown;
    uint32_t            m_nChar;

    uint32_t            m_nHotVk, m_nfsModifiers;

};

#endif // !defined(AFX_HOTKEYCTRLEX_H__C50B3FA5_4505_4569_9A4E_A1C5CDC6C311__INCLUDED_)
