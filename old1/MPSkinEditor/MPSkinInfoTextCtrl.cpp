// MPSkinInfoCtrl.cpp: implementation of the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "MPSkinInfoTextCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrl, "InfoText")

CMPSkinInfoTextCtrl::CMPSkinInfoTextCtrl()
{
    m_strText = "Artist - Title - ZikiPlayer";
    m_msgNeed = UO_MSG_WANT_LBUTTON;
    m_nTimerIDHideInfo = 0;
}

CMPSkinInfoTextCtrl::~CMPSkinInfoTextCtrl()
{
    if (m_nTimerIDHideInfo != 0)
        m_pSkin->unregisterTimerObject(this, m_nTimerIDHideInfo);
}
void CMPSkinInfoTextCtrl::onCreate()
{
    CSkinScrollText::onCreate();
}

void CMPSkinInfoTextCtrl::onTimer(int nId)
{
    if (nId == m_nTimerIDHideInfo)
    {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDHideInfo);
        m_nTimerIDHideInfo = 0;
        invalidate();
    }
    else
        CSkinScrollText::onTimer(nId);
}

void CMPSkinInfoTextCtrl::draw(CRawGraph *canvas)
{
    CSkinScrollText::draw(canvas);
//     if (m_imgBk.isValid())
//     {
//         if (m_BltMode == BLT_COPY)
//         {
//             m_imgBk.blt(canvas, m_rcObj.left, m_rcObj.top);
//         }
//         else if (m_BltMode == BLT_TILE)
//         {
//             m_imgBk.tileBlt(canvas, m_rcObj.left, m_rcObj.top,
//                 m_rcObj.width(), m_rcObj.height());
//         }
//         else
//         {
//             m_imgBk.stretchBlt(canvas, m_rcObj.left, m_rcObj.top,
//                         m_rcObj.width(), m_rcObj.height());
//         }
//     }
// 
//     if (!m_font.isValid())
//     {
//         m_font.create(m_strFontName.c_str(), m_strFontName.c_str(), m_nFontHeight, 
//             m_bFontBold ? FW_BOLD : FW_NORMAL, false, false);
//     }
// 
//     canvas->setFont(&m_font);
//     canvas->setTextColor(m_clrText);
// 
//     CRect        rc = m_rcObj;
// 
//     rc.left += m_nLeftMargin;
//     canvas->drawText(m_strText.c_str(), (int)m_strText.size(), rc);
}

