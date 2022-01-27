#pragma once

//
// CSkinContainerCtrl is the super class of CSkinListCtrl or CSkinEditCtrl.
//
class CSkinScrollFrameCtrlBase : public CSkinContainer, public IScrollNotify
{
public:
    CSkinScrollFrameCtrlBase();
    virtual ~CSkinScrollFrameCtrlBase();

    void onCreate();

    void onSize();

    virtual void onKillFocus();
    virtual void onSetFocus();

    virtual void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt);

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) { }
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) { }

protected:
    void createFrameCtrl();

    void createVertScrollbar();
    void createHorzScrollbar();

protected:
    bool                    m_bEnableBorder;
    bool                    m_bHorzScrollBar, m_bVertScrollBar;
    int                        m_nVertScrollBarId, m_nHorzScrollBarId;        // If this is set, don't create dynamic scroll bar control.
    IScrollBar                *m_pVertScrollBar, *m_pHorzScrollBar;
    CUIObject                *m_pObjVertScrollBar, *m_pObjHorzScrollBar;
    CUIObject                *m_pObjFrame;

    // The size and position of control area
    int                        m_nWidthVertScrollBar, m_nHeightHorzScrollBar;

};
