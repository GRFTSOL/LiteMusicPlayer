// SESkinFactory.h: interface for the CSESkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_)
#define AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSESkinFactory : public CSkinFactory  
{
public:
    CSESkinFactory();
    virtual ~CSESkinFactory();

    virtual void quit();

    virtual CUIObject * createUIObject(CSkinWnd *pSkin, cstr_t szClassName);

    virtual CSkinWnd *newSkinWnd(cstr_t szSkinWndName, bool bMainWnd);

    virtual bool loadMenu(CMenu **ppMenu, cstr_t szMenu);

    virtual void onLanguageChanged();

    virtual CSkinFileXML *getSkinFile() { return &m_skinFile; }

    CDynamicCmds &getDynamicCmds() { return m_dynamicCmds; }
    CDynamicCtrls &getDynamicCtrls() { return m_dynamicCtrls; }

};

#endif // !defined(AFX_SESKINFACTORY_H__0917B794_32AC_4902_8B2F_63D193E7EB39__INCLUDED_)
