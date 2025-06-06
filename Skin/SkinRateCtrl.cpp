#include "Skin.h"
#include "SkinRateCtrl.h"


// set classname
cstr_t CSkinRateCtrl::ms_szClassName = "RateButton";

CSkinRateCtrl::CSkinRateCtrl() {
    m_msgNeed = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

    m_nRating = 0;
    m_nRatingMax = 5;
    m_nRateStarWidth = 0;
}

CSkinRateCtrl::~CSkinRateCtrl() {

}

bool CSkinRateCtrl::onLButtonDown(uint32_t nFlags, CPoint point) {
    assert(m_pSkin);
    updateRatings(point);

    return true;
}

bool CSkinRateCtrl::onLButtonUp(uint32_t nFlags, CPoint point) {
    updateRatings(point);
    m_pSkin->postCustomCommandMsg(m_id);

    return true;
}

void CSkinRateCtrl::updateRatings(CPoint point) {
    int old = m_nRating;
    m_nRating = (point.x - m_rcObj.left + m_nRateStarWidth - 1) / m_nRateStarWidth;
    if (m_nRating > m_nRatingMax) {
        m_nRating = m_nRatingMax;
    } else if (m_nRating < 0) {
        m_nRating = 0;
    }

    if (old != m_nRating) {
        invalidate();
    }
}

cstr_t CSkinRateCtrl::getClassName() {
    return ms_szClassName;
}

void CSkinRateCtrl::draw(CRawGraph *canvas) {
    if (m_nRating < 0 || m_nRating > m_nRatingMax) {
        m_nRating = 0;
    }

    int i;
    int x = m_rcObj.left;

    for (i = 1; i <= m_nRating; i++) {
        m_img.blt(canvas,
            x, m_rcObj.top, m_nRateStarWidth, m_img.m_cy, 0, 0);
        x += m_nRateStarWidth;
    }

    for (i = m_nRating + 1; i <= m_nRatingMax; i++) {
        m_img.blt(canvas,
            x, m_rcObj.top, m_nRateStarWidth, m_img.m_cy, m_nRateStarWidth, 0);
        x += m_nRateStarWidth;
    }
}

bool CSkinRateCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        m_strBmpFile = szValue;
        m_img.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0) {
        if (!getRectValue(szValue, m_img)) {
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
        }
        m_nRateStarWidth = m_img.m_cx / 2;
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinRateCtrl::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpFile.c_str(), m_img);
}
#endif // _SKIN_EDITOR_

void CSkinRateCtrl::setRating(int nRating) {
    m_nRating = nRating;

    invalidate();
}

bool CSkinRateCtrl::isKindOf(cstr_t szClassName) {
    if (CUIObject::isKindOf(szClassName)) {
        return true;
    }

    return strcasecmp(szClassName, ms_szClassName) == 0;
}
