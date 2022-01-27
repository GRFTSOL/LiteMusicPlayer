#pragma once

int saveLyrDialogBox(Window *pWnd);

class CDlgSaveLyrPrompt : public CBaseDialog
{
public:
    CDlgSaveLyrPrompt(cstr_t szInfo) : CBaseDialog(IDD_MSG) { m_strInfo = szInfo; }
    
    bool onInitDialog();

    void onCommand(uint32_t uID, uint32_t nNotifyCode);
    
protected:
    string            m_strInfo;
    
};


