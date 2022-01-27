/********************************************************************
    Created  :    2002/03/27    0:51
    FileName :    SkinScaleImage.h
    Author   :    xhy
    
    Purpose  :    适合于界面中可以伸缩的 水平 或 垂直 的条
*********************************************************************/

#if !defined(AFX_SKNXSCALEIMAGEEX_H__449E9EA2_410B_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_SKNXSCALEIMAGEEX_H__449E9EA2_410B_11D6_B47A_00E04C008BA3__INCLUDED_

#include "UIObject.h"

class CSkinXScaleImage : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinXScaleImage();
    virtual ~CSkinXScaleImage();

public:
    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    string            m_strBmpBkFile;
    CSFImage        m_imgBk;

    // X 方向上的映射关系
    // 1.
    // 从图片的           m_imgBk.m_x ----> m_nScaleXStart
    // 映射到 UIObject的  m_formleft  ----> m_formLeft + m_nScaleXStart - m_imgBk.m_x
    // 2.
    // 从图片的           m_nScaleXStart                             ----> m_nScaleXEnd
    // 映射到 UIObject的  m_formLeft + m_nScaleXStart - m_imgBk.m_x  ----> m_formRight - (m_imgBk.m_x + m_imgBk.m_cx - m_nScaleXEnd)
    // 3.
    // 从图片的           m_nScaleXEnd                             ----> m_imgBk.m_x + m_imgBk.m_cx
    // 映射到 UIObject的  m_formRight - (m_imgBk.m_x + m_imgBk.m_cx - m_nScaleXEnd)  ----> m_formRight
    int                m_nScaleXStart;
    int                m_nScaleXEnd;

    BlendPixMode    m_bpm;

};
///////////////////////////////////////////////////////////////////////////////
//

class CSkinYScaleImage : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinYScaleImage();
    virtual ~CSkinYScaleImage();

public:
    void draw(CRawGraph *canvas);
    bool setProperty(cstr_t szProperty, cstr_t szValue);

#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    CSFImage        m_imgBk;
    string            m_strBmpBkFile;

    // Y 方向上的映射关系
    // 1.
    // 从图片的           m_imgBk.m_y ----> m_nScaleYStart
    // 映射到 UIObject的  m_formtop  ----> m_formTop + m_nScaleYStart - m_imgBk.m_y
    // 2.
    // 从图片的           m_nScaleYStart                             ----> m_nScaleYEnd
    // 映射到 UIObject的  m_formTop + m_nScaleYStart - m_imgBk.m_y  ----> m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)
    // 3.
    // 从图片的           m_nScaleYEnd                             ----> m_imgBk.m_y + m_imgBk.m_cy
    // 映射到 UIObject的  m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)  ----> m_formBottom
    int                m_nScaleYStart;
    int                m_nScaleYEnd;

    BlendPixMode    m_bpm;

};

#endif // !defined(AFX_SKNXSCALEIMAGEEX_H__449E9EA2_410B_11D6_B47A_00E04C008BA3__INCLUDED_)
