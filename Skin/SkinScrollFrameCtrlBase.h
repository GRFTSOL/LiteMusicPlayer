#pragma once

class CSkinScrollFrameCtrlBase : public CSkinContainer, public IScrollNotify {
public:
    CSkinScrollFrameCtrlBase();
    virtual ~CSkinScrollFrameCtrlBase();

    void onCreate() override;

    void onSize() override;

    virtual void onKillFocus() override;
    virtual void onSetFocus() override;

    virtual void onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) override;

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    virtual void onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override { }
    virtual void onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) override { }

protected:
    void createFrameCtrl();

    void createVertScrollbar();
    void createHorzScrollbar();

protected:
    bool                        m_bEnableBorder;

    // If this is set, don't create dynamic scroll bar control.
    bool                        m_bHorzScrollBar, m_bVertScrollBar;
    int                         m_nVertScrollBarId, m_nHorzScrollBarId;
    IScrollBar                  *m_pVertScrollBar, *m_pHorzScrollBar;
    CUIObject                   *m_pObjVertScrollBar, *m_pObjHorzScrollBar;
    CUIObject                   *m_pObjFrame;

    // The size and position of control area
    int                         m_nWidthVertScrollBar, m_nHeightHorzScrollBar;

};
