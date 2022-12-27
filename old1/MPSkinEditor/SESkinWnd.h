// SESkinWnd.h: interface for the CSESkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_)
#define AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum SkinNotifyMsg
{
    SNM_UPDATEPROPERTYITEM        = WM_USER + 1,        // wparam: propname, lparam: value

};

interface ISESkinNotify
{
    virtual void onSkinWndNameChanged() = 0;
    virtual void onUIObjNameChanged(CUIObject *pObj) = 0;
    virtual void onUIObjectListChanged() = 0;
    virtual void onUIObjFocusChanged() = 0;
    virtual void onFocusUIObjPropertyChanged(cstr_t szPropertyName, cstr_t szNewValue) = 0;
    virtual void trackPopupMenu(HMENU hMenu, int x, int y) = 0;
};

class CSESkinWnd : public CSkinWnd  
{
public:
    CSESkinWnd();
    virtual ~CSESkinWnd();

    int create(cstr_t szClassName, cstr_t szCaption, CSkinFactory *pSkinFactory, cstr_t szSkinWndName, Window *pWndParent);

    virtual void newSkin(cstr_t szSkinWndName);

    virtual int openSkin(cstr_t szSkinWndName);

    virtual int copyFrom(cstr_t szSkinWndName, CSESkinWnd *pSkinWndSrc);

    void onSkinLoaded();

    bool isUIObjectExist(CUIObject *pObj);

    int insertUIObject(CUIObject *pObj);


    void setNOFocusUIObj();

    bool removeFocusUIObject();

    void onKeyDown(uint32_t nChar, uint32_t nFlags);
    void onLButtonDown(uint32_t nFlags, CPoint point);
    void onMouseMove(uint32_t nFlags, CPoint point);

    void onPaint(CRawGraph *canvas, CRect *rcClip);
    void onContexMenu(int xPos, int yPos);
    void onRButtonUp(uint32_t nFlags, CPoint point);

    LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    void enumProperties(CUIObjProperties &listProperties);

    void zoomIn();
    void zoomOut();

    void setNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }
    ISESkinNotify *getNotify() { return m_pNotify; }

protected:
    HMENU        m_hMenuContext;
    ISESkinNotify    *m_pNotify;

    // other properties...
    string        m_strCmdHandler;
    string        m_strContextMenu;
    bool        m_bEnableVolumeSwitch;
    bool        m_bIndividualProperties;

    bool        m_bEnableTransparentFeatureEditing;
    bool        m_bTranslucencyLayeredEditing;
    
};

#endif // !defined(AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_)
