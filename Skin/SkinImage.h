/********************************************************************
    Created  :    2002/03/03    13:46
    FileName :    SkinImage.h
    Author   :    xhy
    
    Purpose  :    SKIN 中的可伸缩图片控件类
*********************************************************************/

#if !defined(AFX_SKNIMAGE_H__EAA358E2_2EAA_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_SKNIMAGE_H__EAA358E2_2EAA_11D6_B47A_00E04C008BA3__INCLUDED_

#include "UIObject.h"


class CSkinNStatusImage : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinNStatusImage();
    virtual ~CSkinNStatusImage();

public:
    void onCreate();

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    void setMaxStat(int nMaxStat);
    void destroy();

protected:
    struct StatImg
    {
        string            strImgFile;
        CSFImage        img;
    };

    typedef vector<StatImg *>        V_STATIMG;

    V_STATIMG            m_vStImages;
    int                    m_nCurStatus;

    BLT_MODE            m_BltMode;
    BlendPixMode        m_bpm;

};


class CSkinImage : public CSkinNStatusImage  
{
UIOBJECT_CLASS_NAME_DECLARE(CSkinNStatusImage)
public:
    CSkinImage();
    virtual ~CSkinImage();

public:
    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

};

class CSkinActiveImage : public CSkinImage
{
UIOBJECT_CLASS_NAME_DECLARE(CSkinImage)
public:
    CSkinActiveImage();
    virtual ~CSkinActiveImage();

    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

};

#endif // !defined(AFX_SKNIMAGE_H__EAA358E2_2EAA_11D6_B47A_00E04C008BA3__INCLUDED_)
