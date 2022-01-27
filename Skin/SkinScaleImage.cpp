/********************************************************************
    Created  :    2002/03/27    0:45
    FileName :    SkinScaleImage.cpp
    Author   :    xhy
    
    Purpose  :    适合于界面中可以伸缩的 水平 或 垂直 的条
*********************************************************************/

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinScaleImage.h"

UIOBJECT_CLASS_NAME_IMP(CSkinXScaleImage, "HStretchImage")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinXScaleImage::CSkinXScaleImage()
{
    m_nScaleXStart = 0;
    m_nScaleXEnd = 0;
    m_bpm = BPM_BLEND;
}

CSkinXScaleImage::~CSkinXScaleImage()
{

}

void CSkinXScaleImage::draw(CRawGraph *canvas)
{
    int            nWidthLeft;
    int            nWidthRight;

    nWidthLeft = m_nScaleXStart - m_imgBk.x();
    nWidthRight = m_imgBk.x() + m_imgBk.width() - m_nScaleXEnd;

    // 画出第一部分
    // X 方向上的映射关系
    // 1.
    // 从图片的           m_imgBk.x() ----> m_nScaleXStart
    // 映射到 UIObject的  m_formleft  ----> m_formLeft + m_nScaleXStart - m_imgBk.x()
	m_imgBk.blt(canvas, 
		m_rcObj.left, m_rcObj.top,
		nWidthLeft, m_imgBk.m_cy,
		m_imgBk.m_x, m_imgBk.m_y, m_bpm
	);


    // 2.
    // 从图片的           m_nScaleXStart                             ----> m_nScaleXEnd
    // 映射到 UIObject的  m_formLeft + m_nScaleXStart - m_imgBk.x()  ----> m_formRight - (m_imgBk.x() + m_imgBk.m_cx - m_nScaleXEnd)
	m_imgBk.stretchBlt(canvas, 
		m_rcObj.left + nWidthLeft, m_rcObj.top,
			m_rcObj.width() - nWidthLeft - nWidthRight, m_rcObj.height(),
			m_nScaleXStart, m_imgBk.m_y,
			m_nScaleXEnd - m_nScaleXStart, m_imgBk.m_cy, m_bpm
	);

    // 3.
    // 从图片的           m_nScaleXEnd                             ----> m_imgBk.x() + m_imgBk.m_cx
    // 映射到 UIObject的  m_formRight - (m_imgBk.x() + m_imgBk.m_cx - m_nScaleXEnd)  ----> m_formRight
	m_imgBk.blt(canvas, 
		m_rcObj.right - nWidthRight, m_rcObj.top,
		nWidthRight, m_imgBk.m_cy,
		m_nScaleXEnd, m_imgBk.m_y, m_bpm
	);
}

bool CSkinXScaleImage::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CUIObject::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0)
    {
        m_strBmpBkFile = szValue;
        m_imgBk.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    }
    else if (strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0)
    {
        if (!getRectValue(szValue, m_imgBk))
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
    }
    else if (strcasecmp(szProperty, "StretchStartX") == 0 ||
        strcasecmp(szProperty, "ImageScaleXStart") == 0)
        m_nScaleXStart = atoi(szValue);
    else if (strcasecmp(szProperty, "StretchEndX") == 0 || 
        strcasecmp(szProperty, "ImageScaleXEnd") == 0)
        m_nScaleXEnd = atoi(szValue);
    else if (isPropertyName(szProperty, "BlendPixMode"))
    {
        m_bpm = blendPixModeFromStr(szValue);
    }
    else
        return false;

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinXScaleImage::enumProperties(CUIObjProperties &listProperties)
{
    CUIObject::enumProperties(listProperties);

    vector<string>        vExtra;
    char                szBuff[64];

    _itot_s(m_nScaleXStart, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchStartX");
    vExtra.push_back(szBuff);

    _itot_s(m_nScaleXEnd, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchEndX");
    vExtra.push_back(szBuff);

    listProperties.addPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);
    listProperties.addPropBlendPixMode("BlendPixMode", m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_

//////////////////////////////////////////////////////////////////////
// CSkinYScaleImage

UIOBJECT_CLASS_NAME_IMP(CSkinYScaleImage, "VStretchImage")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinYScaleImage::CSkinYScaleImage()
{
    m_nScaleYStart = 0;
    m_nScaleYEnd = 0;
    m_bpm = BPM_BLEND;
}

CSkinYScaleImage::~CSkinYScaleImage()
{

}

void CSkinYScaleImage::draw(CRawGraph *canvas)
{
    int nHeightTop = m_nScaleYStart - m_imgBk.y();
    int nHeightBottom = m_imgBk.y() + m_imgBk.height() - m_nScaleYEnd;

    // 画出第一部分
    // Y 方向上的映射关系
    // 1.
    // 从图片的           m_imgBk.m_y ----> m_nScaleYStart
    // 映射到 UIObject的  m_formtop  ----> m_formTop + m_nScaleYStart - m_imgBk.m_y
	m_imgBk.blt(canvas, 
		m_rcObj.left, m_rcObj.top,
		m_imgBk.m_cx, nHeightTop,
		m_imgBk.m_x, m_imgBk.m_y, m_bpm
	);

    // 2.
    // 从图片的           m_nScaleYStart                             ----> m_nScaleYEnd
    // 映射到 UIObject的  m_formTop + m_nScaleYStart - m_imgBk.m_y  ----> m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)
	m_imgBk.stretchBlt(canvas, 
		m_rcObj.left, m_rcObj.top + nHeightTop,
		m_rcObj.width(), m_rcObj.height() - nHeightTop - nHeightBottom,
		m_imgBk.m_x, m_nScaleYStart,
		m_imgBk.m_cx, m_nScaleYEnd - m_nScaleYStart, m_bpm
	);

    // 3.
    // 从图片的           m_nScaleYEnd                             ----> m_imgBk.m_y + m_imgBk.m_cy
    // 映射到 UIObject的  m_formBottom - (m_imgBk.m_y + m_imgBk.m_cy - m_nScaleYEnd)  ----> m_formBottom
	m_imgBk.blt(canvas, 
		m_rcObj.left, m_rcObj.bottom  - nHeightBottom,
		m_imgBk.m_cx, nHeightBottom,
		m_imgBk.m_x, m_nScaleYEnd, m_bpm
	);
}

bool CSkinYScaleImage::setProperty(cstr_t szProperty, cstr_t szValue)
{
    assert(m_pSkin);

    if (CUIObject::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0)
    {
        m_strBmpBkFile = szValue;
        m_imgBk.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    }
    else if (strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0)
    {
        if (!getRectValue(szValue, m_imgBk))
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
    }
    else if (strcasecmp(szProperty, "StretchStartY") == 0 ||
        strcasecmp(szProperty, "ImageScaleYStart") == 0)
        m_nScaleYStart = atoi(szValue);
    else if (strcasecmp(szProperty, "StretchEndY") == 0 || 
        strcasecmp(szProperty, "ImageScaleYEnd") == 0)
        m_nScaleYEnd = atoi(szValue);
    else if (isPropertyName(szProperty, "BlendPixMode"))
    {
        m_bpm = blendPixModeFromStr(szValue);
    }
    else
        return false;

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinYScaleImage::enumProperties(CUIObjProperties &listProperties)
{
    CUIObject::enumProperties(listProperties);

    vector<string>        vExtra;
    char                szBuff[64];

    _itot_s(m_nScaleYStart, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchStartY");
    vExtra.push_back(szBuff);

    _itot_s(m_nScaleYEnd, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchEndY");
    vExtra.push_back(szBuff);

    listProperties.addPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);
    listProperties.addPropBlendPixMode("BlendPixMode", m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_
