// MPPPCWorkArea.h: interface for the CMPPPCWorkArea class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPPPCWORKAREA_H__610B390D_D130_4F73_BD21_03A2B7409487__INCLUDED_)
#define AFX_MPPPCWORKAREA_H__610B390D_D130_4F73_BD21_03A2B7409487__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMPPPCWorkArea : public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    // working area type
    enum WORK_AREA
    {
        WA_LYC,        // lyrics
        WA_PL,        // playlist
        WA_LYC_PL,    // playlist and lyrics
    };

public:
    CMPPPCWorkArea();
    virtual ~CMPPPCWorkArea();

    bool setProperty(cstr_t szProperty, cstr_t szValue);
    void enumProperties(CUIObjProperties &listProperties);

protected:
    string                m_strDivideImgFile;
    CSFImage            m_imgDivide;
    CRect                m_rcDivideImg;
    string                m_strDivideImgBltMode;
    VecStrings                m_vProperties;

    string                m_strLyrDisplayStyle;

    WORK_AREA            m_workArea;

};

#endif // !defined(AFX_MPPPCWORKAREA_H__610B390D_D130_4F73_BD21_03A2B7409487__INCLUDED_)
