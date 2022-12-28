#pragma once

#ifndef MPlayerUI_win32_DlgBatchProcessEmbeddedLyrics_h
#define MPlayerUI_win32_DlgBatchProcessEmbeddedLyrics_h


class CDlgBatchProcessEmbeddedLyrics : public CBaseDialog {
public:
    enum {
        COL_OPRATION,
        COL_SONG,
        COL_RESULT,
        COL_EMBEDDED_TYPE,
    };

    CDlgBatchProcessEmbeddedLyrics();
    virtual ~CDlgBatchProcessEmbeddedLyrics();

    bool onInitDialog();

    void onOK();

protected:
    CWidgetListCtrl             m_listCtrl;

};

#endif // !defined(MPlayerUI_win32_DlgBatchProcessEmbeddedLyrics_h)
