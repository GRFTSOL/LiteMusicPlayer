#include "MPlayerApp.h"
#include "DlgSaveLyrPrompt.h"

int saveLyrDialogBox(Window *pWnd)
{
    static bool bInModal = false;
    if (!bInModal)
    {
        bInModal = true;
        
        string        strMessage = CStrPrintf(_TLT("The lyrics of the %s file have changed."), 
                                            fileGetName(g_LyricData.getSongFileName())).c_str();
        strMessage += "\r\n\r\n";
        strMessage += _TLT("Do you want to save the changes?");

        int nRet = pWnd->messageOut(strMessage.c_str(), MB_YESNOCANCEL, SZ_APP_NAME);
        
        bInModal = false;
        return nRet;
    }
    
    return IDCANCEL;
}

