
#pragma once

class CDlgSaveEmbeddedLyrics : public CBaseDialog  
{
public:
    CDlgSaveEmbeddedLyrics();
    virtual ~CDlgSaveEmbeddedLyrics();

    int doModal(Window *pWnd);

    virtual bool onInitDialog();

    virtual void onOK();

    enum {
        COL_DESC = 0,
        COL_NAME,
    };

protected:
    void addEmbeddedItem(VecStrings &vAlreadyHave, cstr_t szToAdd);

    int doSaveAction(VecStrings &vLyrNames, Window *pWnd);

protected:
    string                m_strMediaUrl;
    string            m_bufLyrics;

    CWidgetListCtrl        m_ctrlListLyrics;
    uint32_t                m_nDefaultEmbededLST;
    bool                m_bHasEmbeddedLyrAlready;

};
