#pragma once

#ifndef MPlayerUI_mac_DlgBatchProcessEmbeddedLyrics_h
#define MPlayerUI_mac_DlgBatchProcessEmbeddedLyrics_h


class CDlgBatchProcessEmbeddedLyrics {
public:
    enum {
        COL_OPRATION,
        COL_SONG,
        COL_RESULT,
        COL_EMBEDDED_TYPE,
    };

    CDlgBatchProcessEmbeddedLyrics() { }
    virtual ~CDlgBatchProcessEmbeddedLyrics() { }

    int doModal(Window *pWnd) { return IDOK; }

};

#endif // !defined(MPlayerUI_mac_DlgBatchProcessEmbeddedLyrics_h)
