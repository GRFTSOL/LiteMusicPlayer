#pragma once

#ifndef MPlayerUI_MPCommonCmdHandler_h
#define MPlayerUI_MPCommonCmdHandler_h


#include "MPEventsDispatcher.h"
#include "LyrDisplayClrListWnd.h"


class CMPCommonCmdHandler : public ISkinCmdHandler {
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
    string                      m_strSectName;
    EventType                   m_etDispSettings;

    CLyrDisplayClrListWnd       m_popupHighClrListWnd;

};

#endif // !defined(MPlayerUI_MPCommonCmdHandler_h)
