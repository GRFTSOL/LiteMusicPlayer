#include "Skin.h"
#include "SkinScrollText.h"


#define TIMER_SPAN_SCROLL   30
#define WIDTH_TXT_STILL            (2000 / TIMER_SPAN_SCROLL)

UIOBJECT_CLASS_NAME_IMP(CSkinScrollText, "ScrollText")

CSkinScrollText::CSkinScrollText() {
    m_nTimerIDScroll = 0;
    m_nPosScroll = WIDTH_TXT_STILL;
    m_nWidthText = 0;
    m_bToLeft = true;
}

CSkinScrollText::~CSkinScrollText() {

}
//
// bool CSkinScrollText::onLButtonUp(uint32_t nFlags, CPoint point)
// {
//     if (!m_enable)
//         return false;
//
//     if (m_id != ID_INVALID)
//         m_pSkin->postCustomCommandMsg(m_id);
//
//     return true;
// }

void CSkinScrollText::draw(CRawGraph *canvas) {
    CRawGraph::CClipBoxAutoRecovery clipRecover(canvas);
    canvas->setClipBoundBox(m_rcObj);

    CUIObject::draw(canvas);

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    CRect rc = m_rcObj;

    rc.left += m_nLeftMargin;
    rc.top = m_rcObj.top + (m_rcObj.height() - m_font.getHeight()) / 2;
    rc.bottom = rc.top + m_font.getHeight();

    CSize size;
    canvas->getTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size);
    size.cx += 3;
    if (m_nWidthText != size.cx) {
        m_nWidthText = size.cx;
        m_nPosScroll = 0;
        m_bToLeft = true;
    }

    // set timer to scroll
    if (m_nWidthText > rc.width() && m_nTimerIDScroll == 0) {
        m_nTimerIDScroll = m_pSkin->registerTimerObject(this, TIMER_SPAN_SCROLL);
    }

    // kill timer
    if (m_nWidthText <= rc.width() && m_nTimerIDScroll != 0) {
        m_pSkin->unregisterTimerObject(this, m_nTimerIDScroll);
        m_nTimerIDScroll = 0;
        m_nPosScroll = -WIDTH_TXT_STILL;
    }

    int nLeftClip = 0;
    if (m_nTimerIDScroll != 0 && m_nPosScroll > 0) {
        if (m_nPosScroll + rc.width() >= m_nWidthText) {
            nLeftClip = m_nWidthText - rc.width();
            rc.left -= m_nWidthText - rc.width();
        } else {
            rc.left -= m_nPosScroll;
            nLeftClip = m_nPosScroll;
        }
    }

    if (m_font.isOutlined()) {
        canvas->drawTextClipOutlined(m_strText.c_str(), (int)m_strText.size(), rc, m_font.getTextColor(m_enable), m_font.getColorOutlined(), nLeftClip);
    } else {
        canvas->setTextColor(m_font.getTextColor(m_enable));
        canvas->drawTextClip(m_strText.c_str(), (int)m_strText.size(), rc, nLeftClip);
    }
    // canvas->drawText(m_strText.c_str(), (int)m_strText.size(), &rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
}
//
// bool CSkinScrollText::setProperty(cstr_t szProperty, cstr_t szValue)
// {
//     if (CSkinStaticText::setProperty(szProperty, szValue))
//         return true;
//     else
//         return false;
//
//     return true;
// }
//
// void CSkinScrollText::enumProperties(CUIObjProperties &listProperties)
// {
//     CSkinStaticText::enumProperties(listProperties);
// }
//
// void CSkinScrollText::onCreate()
// {
// //     m_nTimerIDScroll = m_pSkin->registerTimerObject(this, TIMER_SPAN_SCROLL);
// }

bool CSkinScrollText::onLButtonUp(uint32_t nFlags, CPoint point) {
    return true;
}

bool CSkinScrollText::onLButtonDown(uint32_t nFlags, CPoint point) {
    return true;
}

void CSkinScrollText::onTimer(int nId) {
    if (nId == m_nTimerIDScroll) {
        if (m_bToLeft) {
            m_nPosScroll--;
            if (m_nPosScroll <= -WIDTH_TXT_STILL) {
                m_bToLeft = false;
                m_nPosScroll = 0;
            }
        } else {
            m_nPosScroll++;
            if (m_nPosScroll >= (m_nWidthText - m_rcObj.width() - m_nLeftMargin) + WIDTH_TXT_STILL) {
                m_bToLeft = true;
                m_nPosScroll = (m_nWidthText - m_rcObj.width() - m_nLeftMargin);
            }
        }

        invalidate();
    }
}
