// MPCommonCmdHandler.h: interface for the CMPCommonCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_)
#define AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPEventsDispatcher.h"
#include "LyrDisplayClrListWnd.h"

class CMPCommonCmdHandler : public ISkinCmdHandler  
{
public:
    CMPCommonCmdHandler(bool bFloatingLyr = false);
    virtual ~CMPCommonCmdHandler();

    // if the command id is processed, return true.
    virtual bool onCommand(int nId);
    virtual bool onCustomCommand(int nID);
    virtual bool onUIObjNotify(IUIObjNotify *pNotify);

    virtual bool getChecked(uint32_t nID, bool &bChecked);
    virtual bool getRadioChecked(vector<uint32_t> &vIDs, uint32_t &nIDChecked);

    static bool saveAsLyricsFile(Window *pWndParent, MLFileType DefFileType);
    static bool saveCurrentLyrics(CSkinWnd *pSkinWnd, bool bDispatchOnSave);

protected:
    bool onCommandCharEncoding(int nCmdId);

    CLyricsLines &getDisplayLyrics();

protected:
    string            m_strSectName;
    EventType        m_etDispSettings;

    CLyrDisplayClrListWnd    m_popupHighClrListWnd;

};

#endif // !defined(AFX_MPCOMMONCMDHANDLER_H__C770324D_2964_450B_AB1C_5DD1C118B0FD__INCLUDED_)
