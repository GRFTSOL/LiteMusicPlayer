// DebugOutputWnd.h: interface for the CDebugOutputWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEBUGOUTPUTWND_H__3CF83007_8EDB_4DCE_8BC2_B970595DE4AF__INCLUDED_)
#define AFX_DEBUGOUTPUTWND_H__3CF83007_8EDB_4DCE_8BC2_B970595DE4AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDebugOutputWnd : public Window  
{
public:
    CDebugOutputWnd();
    virtual ~CDebugOutputWnd();

    bool create();

    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

    virtual void onSize(int cx, int cy);

protected:
    CWidgetEditBox        m_edit;

};

#endif // !defined(AFX_DEBUGOUTPUTWND_H__3CF83007_8EDB_4DCE_8BC2_B970595DE4AF__INCLUDED_)
