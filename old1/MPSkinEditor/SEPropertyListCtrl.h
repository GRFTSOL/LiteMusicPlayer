// SEPropertyListCtrl.h: interface for the CSEPropertyListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_)
#define AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SESkinWnd.h"

class CSESkinWnd;

class CSESkinPropertyAdapter
{
public:
    CSESkinPropertyAdapter()
    {
        m_hWndCommand = nullptr;
        m_pwndSkin = nullptr;
        m_pObj = nullptr;
    }

    void updatePropertyToUI(cstr_t szName, cstr_t szValue)
    {
        sendMessage(m_hWndCommand, SNM_UPDATEPROPERTYITEM, (WPARAM)szName, (LPARAM)szValue);
    }

    void updatePropertyToSkin()
    {
        PostMessage(m_hWndCommand, WM_COMMAND, IDC_PROPERTY_SAVE, 0);
    }

    void updatePropertyToSkin(cstr_t szName, cstr_t szValue)
    {
        PostMessage(m_hWndCommand, WM_COMMAND, IDC_PROPERTY_SAVE, 0);
    }

    void trackPopupMenu(HMENU hMenu, int x, int y)
    {
        ::trackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, x, y, 0, m_hWndCommand, nullptr);
    }

    bool isSrcChanged(CSESkinWnd *pwndSkin, CUIObject *pObj)
    {
        return m_pwndSkin != pwndSkin || m_pObj != pObj;
    }

    void setSrc(CSESkinWnd *pwndSkin, CUIObject *pObj, HWND hWndCmd)
    {
        m_pwndSkin = pwndSkin;
        m_pObj = pObj;
        m_hWndCommand = hWndCmd;
    }

    HWND            m_hWndCommand;
    CSESkinWnd        *m_pwndSkin;
    CUIObject        *m_pObj;

};

extern CSESkinPropertyAdapter        g_seSkinPropertyAdapter;

struct SEPLItem
{
    SEPLItem()
    {
        pBrush = nullptr;
        nOptSelected = 0;
        nAlignment = DT_LEFT;
        nWidth = 0;
        bComboEditable = false;
        bComboSorted = false;
        LogFont.lfHeight = 12;
    }
    CUIObjProperty        prop;
    CBrush                *pBrush;
    int                    nWidth;
    int                    nAlignment;
    int                    nOptSelected;
    bool                bComboEditable;
    bool                bComboSorted;
    LOGFONT                LogFont;

    bool isEdit()
    {
        return prop.valueType == CUIObjProperty::VT_INT ||
            prop.valueType == CUIObjProperty::VT_STR ||
            prop.valueType == CUIObjProperty::VT_RECT ||
            prop.valueType == CUIObjProperty::VT_VAR_INT;
    }
};

/////////////////////////////////////////////////////////////////////////////
// CComboButton window
class CComboButton : public CButton
{
    void drawTriangle(CDC* pDC, CRect Rect);

// Construction
public:
    bool    create( CRect Rect, CWnd* pParent, uint32_t uID);
    CComboButton();

// Attributes
public:
    CPen*        m_pBkPen;
    CPen*        m_pGrayPen;
    CBrush*        m_pBkBrush;  
    CBrush*        m_pBlackBrush;

// Operations
public:

    virtual void drawItem(LPDRAWITEMSTRUCT lpDrawItemStruct );
    virtual void measureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CComboButton)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CComboButton();

    // Generated message map functions
protected:
    //{{AFX_MSG(CComboButton)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

class CSEPropertyListCtrl : public CListBox  
{
    DECLARE_MESSAGE_MAP()

public:
    CSEPropertyListCtrl();
    virtual ~CSEPropertyListCtrl();

private:
    // Helper Functions
    void    drawItem(CDC* pDC, CRect itemRect, bool bSelected);
    void    drawPropertyText(CDC* pDC, CRect itemRect);
    void    createControl(CUIObjProperty::VALUE_TYPE valueType);
    void    hideControls();

    void onPropertyChanged(int nIndex);

// Operations
public:
    // GUI Functions
    void            setFont(CFont* pFont);
    void            setBkColor(COLORREF crColor);
    void            setPropertyBkColor(COLORREF crColor);
    void            setHighlightColor(COLORREF crColor);
    void            setLineStyle(COLORREF crColor, int nStyle = PS_SOLID);
    inline    void    setBoldSelection(bool bBoldSelection)            { m_bBoldSelection = bBoldSelection; };
    inline    void    setTextColor(COLORREF crColor)                    { m_crTextColor = crColor; };
    inline    void    setTextHighlightColor(COLORREF crColor)            { m_crTextHighlightColor = crColor; };
    inline    void    setPropertyTextColor(COLORREF crColor)            { m_crPropertyTextColor = crColor; };

    // add the data
    bool addProperty(CUIObjProperty &prop, int nAlignment = DT_LEFT);
    bool addProperties(CUIObjProperties &properties, int nAlignment = DT_LEFT);

    bool setProperty(cstr_t szProp, cstr_t szValue);

    CUIObjProperty *getProperty(int nIndex);
    void getProperties(vector<string> &properties);

    void resetContent();
    
// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSEPropertyListCtrl)
    public:
    virtual void drawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void measureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    //}}AFX_VIRTUAL

protected:
    int getTextExtent(cstr_t szText);

    void onChooseIDClick();
    void onImagePropertyClick();
    void onFontPropertyClick();
    void onPathPropertyClick();

    // Generated message map functions
protected:
    //{{AFX_MSG(CSEPropertyListCtrl)
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg HBRUSH CtlColor(CDC* pDC, uint32_t nCtlColor);
    afx_msg void OnSelchange();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, uint32_t nCtlColor);
    afx_msg void OnDblclk();
    afx_msg void OnEditLostFocus();
    afx_msg void OnEditChange();
    afx_msg void OnButtonClick();
    afx_msg void OnComboBoxClick();
    afx_msg void OnSelChange();
    afx_msg void OnListboxLostFocus();
    afx_msg void onLButtonDown(uint32_t nFlags, CPoint point);
    afx_msg void onVScroll(uint32_t nSBCode, uint32_t nPos, CScrollBar* pScrollBar);
    //}}AFX_MSG

protected:
    typedef vector<SEPLItem>    V_SEPLITEM;
    V_SEPLITEM                m_vProperies;
    SEPLItem                *m_pCurItem, *m_pCurDrawItem;

    int                m_nWidestItem;
    bool            m_bDeleteFont;
    bool            m_bBoldSelection;
    bool            m_bChanged;
    CPen*            m_pBorderPen;
    CRect            m_CurRect;
    CFont*            m_pTextFont;
    CFont*            m_pSelectedFont;
    CFont*            m_pCurFont;
    CString            m_csText;
    CBrush*            m_pCurBrush;
    CBrush*            m_pBkBrush;
    CBrush*            m_pBkHighlightBrush;
    CBrush*            m_pBkPropertyBrush;

    CButton            *m_pButton;
    CComboButton*    m_pComboButton;
    CListBox*        m_pListBox;

    // Controls
    CEdit*            m_pEditWnd;

    COLORREF        m_crBorderColor;
    COLORREF        m_crBkColor;
    COLORREF        m_crTextColor;
    COLORREF        m_crTextHighlightColor;
    COLORREF        m_crHighlightColor;
    COLORREF        m_crPropertyBkColor;
    COLORREF        m_crPropertyTextColor;

};

#endif // !defined(AFX_SEPROPERTYLISTCTRL_H__E86DCDE7_09BC_4684_A7B5_896177DD9BDA__INCLUDED_)
