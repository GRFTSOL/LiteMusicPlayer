#include "MPlayerApp.h"
#include "DlgSaveLyrPrompt.h"

int saveLyrDialogBox(Window *pWnd)
{
    static bool bInModal = false;
    if (!bInModal)
    {
        bInModal = true;
        
        int nRet = g_profile.getInt("SaveLyrPrompt", IDCANCEL);
        if (nRet == IDCANCEL)
        {
            string        strMessage = CStrPrintf(_TLT("The lyrics of the %s file have changed."), 
                                                fileGetName(g_LyricData.getSongFileName())).c_str();
            strMessage += "\r\n\r\n";
            strMessage += _TLT("Do you want to save the changes?");
            
            CDlgSaveLyrPrompt dlg(strMessage.c_str());
            nRet = dlg.doModal(pWnd);
        }
        
        bInModal = false;
        return nRet;
    }
    
    return IDCANCEL;
}

bool CDlgSaveLyrPrompt::onInitDialog()
{
    if (!CBaseDialog::onInitDialog())
        return false;
    
    setDlgItemText(IDC_MSG, m_strInfo.c_str());
    
    return true;
}

void CDlgSaveLyrPrompt::onCommand(uint32_t uID, uint32_t nNotifyCode)
{
    switch (uID)
    {
        case IDC_NO_PROMPT:
            break;
        case IDYES:
        case IDNO:
            if (isButtonChecked(IDC_NO_PROMPT))
                g_profile.writeInt("SaveLyrPrompt", uID);
        case IDCANCEL:
            EndDialog(m_hWnd, uID);
            break;
    }
}
