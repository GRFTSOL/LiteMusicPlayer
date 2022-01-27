// ImgList.cpp: implementation of the CSFImgList class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "SFImage.h"
#include "SFImgList.h"

#ifndef ILC_COLOR32
#define ILC_COLOR32        0
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSFImgList::CSFImgList()
{
    m_nItemCx = 0;
}

CSFImgList::~CSFImgList()
{
}

bool CSFImgList::load(CSkinFactory *pSkinFactory, cstr_t szImage, int nCx)
{
    destroy();

    if (!CSFImage::loadFromSRM(pSkinFactory, szImage))
        return false;

    m_nItemCx = nCx;

    return true;
}

bool CSFImgList::draw(CRawGraph *canvas, int nIndex, int x, int y, int width, int height, BlendPixMode bpm)
{
    if (width <= 0)
        width = m_nItemCx;
    else if (width > m_nItemCx)
        width = m_nItemCx;
    if (height <= 0)
        height = m_cy;
    else if (height > m_cy)
        height = m_cy;

	return blt(canvas, x, y, width, height, m_x + m_nItemCx * nIndex, m_y);
}

int CSFImgList::getIconCx()
{
    return m_nItemCx;
}

int CSFImgList::getIconCy()
{
    return m_cy;
}
