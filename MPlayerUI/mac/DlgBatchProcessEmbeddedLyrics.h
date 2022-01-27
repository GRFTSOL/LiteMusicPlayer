// DlgBatchProcessEmbeddedLyrics.h: interface for the CDlgBatchProcessEmbeddedLyrics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGBATCHSAVEEMBEDDEDLYRICS_H__7C2A5A6E_1CB5_4DA9_B1B0_9B1D64D1BCC9__INCLUDED_)
#define AFX_DlgBatchProcessEmbeddedLyrics_H__7C2A5A6E_1CB5_4DA9_B1B0_9B1D64D1BCC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgBatchProcessEmbeddedLyrics
{
public:
    enum
    {
        COL_OPRATION,
        COL_SONG,
        COL_RESULT,
        COL_EMBEDDED_TYPE,
    };

    CDlgBatchProcessEmbeddedLyrics() { }
    virtual ~CDlgBatchProcessEmbeddedLyrics() { }
    
    int doModal(Window *pWnd) { return IDOK; }

};

#endif // !defined(AFX_DlgBatchProcessEmbeddedLyrics_H__7C2A5A6E_1CB5_4DA9_B1B0_9B1D64D1BCC9__INCLUDED_)
