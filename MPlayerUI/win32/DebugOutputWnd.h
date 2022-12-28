#pragma once

#ifndef MPlayerUI_win32_DebugOutputWnd_h
#define MPlayerUI_win32_DebugOutputWnd_h


class CDebugOutputWnd : public Window {
public:
    CDebugOutputWnd();
    virtual ~CDebugOutputWnd();

    bool create();

    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

    virtual void onSize(int cx, int cy);

protected:
    CWidgetEditBox              m_edit;

};

#endif // !defined(MPlayerUI_win32_DebugOutputWnd_h)
