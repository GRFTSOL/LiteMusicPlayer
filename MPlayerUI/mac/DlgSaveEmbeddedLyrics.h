#pragma once

class CDlgSaveEmbeddedLyrics {
public:
    CDlgSaveEmbeddedLyrics();
    virtual ~CDlgSaveEmbeddedLyrics();

    int doModal(Window *pWnd);

    virtual bool onInitDialog();

    virtual void onOK();

    enum {
        COL_DESC                    = 0,
        COL_NAME,
    };

    void *getControllerHandle() { return m_pDlgController; }

protected:
    void addEmbeddedItem(VecStrings &vAlreadyHave, cstr_t szToAdd);

    int doSaveAction(VecStrings &vLyrNames, Window *pWnd);

protected:
    string                      m_strMediaUrl;
    string                      m_bufLyrics;

    void                        *               m_pDlgController;
    uint32_t                    m_nDefaultEmbededLST;
    bool                        m_bHasEmbeddedLyrAlready;

    Window                      *m_pParentWnd;

};
