// SkinEditorFrame.h: interface for the CSkinEditorFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_)
#define AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSkinEditorFrame : public CMDIChildWnd, public ISESkinNotify, public ISESkinDataItem
{
    DECLARE_DYNCREATE(CSkinEditorFrame)
public:
    CSkinEditorFrame();

    virtual void onSkinWndNameChanged();
    virtual void onUIObjNameChanged(CUIObject *pObj);
    virtual void onUIObjectListChanged();
    virtual void onUIObjFocusChanged();
    virtual void onFocusUIObjPropertyChanged(cstr_t szPropertyName, cstr_t szNewValue);
    virtual void trackPopupMenu(HMENU hMenu, int x, int y);

    virtual void onSave();

// Attributes
public:

// Operations
public:
    void updateToPropertyListCtrl();
    void setPropertiesToUIObj();

    void updatePropertyItem(cstr_t szPropName, cstr_t szValue);
    void updateSkinObjList();

    void insertUIObj(cstr_t szClassName);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSkinEditorFrame)
    public:
    virtual bool preCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Implementation
public:
    // view for the client area of the frame.
    CSESkinWnd        m_wndSkin;
    virtual ~CSkinEditorFrame();
#ifdef _DEBUG
    virtual void assertValid() const;
    virtual void dump(CDumpContext& dc) const;
#endif

    LRESULT windowProc(uint32_t message, WPARAM wParam, LPARAM lParam);

// Generated message map functions
protected:
    //{{AFX_MSG(CSkinEditorFrame)
    afx_msg void OnFileClose();
    afx_msg void onSetFocus(CWnd* pOldWnd);
    afx_msg int onCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void onPaint();
    afx_msg void onDestroy();
    afx_msg void OnInsertBtn();
    afx_msg void OnProperty();
    afx_msg void OnDelete();
    afx_msg void OnInsertFocusBtn();
    afx_msg void OnInsertImg();
    afx_msg void OnInsertFocusImg();
    afx_msg void OnInsertXscaleImg();
    afx_msg void OnInsertYscaleImg();
    afx_msg void OnInsertVscrollbar();
    afx_msg bool OnEraseBkgnd(CDC* pDC);
    afx_msg void OnPropertySave();
    afx_msg void onLButtonDown(uint32_t nFlags, CPoint point);
    afx_msg void OnRoomIn();
    afx_msg void OnRoomOut();
    afx_msg void onKillFocus(CWnd* pNewWnd);
    afx_msg void OnUpdateSkinObjList();
    afx_msg void OnSkinObjListSelChanged();
    afx_msg void OnEditCopy();
    afx_msg void OnEditCut();
    afx_msg void OnEditPaste();
    afx_msg void OnEditUndo();
    afx_msg void OnRedo();
    afx_msg void OnSysCommand(uint32_t nID, LPARAM lParam);
    afx_msg void OnChildActivate();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKINEDITORFRAME_H__03D6D8DB_5D50_4E1F_9A4F_07936F7ACEE4__INCLUDED_)
