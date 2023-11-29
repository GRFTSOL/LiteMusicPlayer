#include "SkinTypes.h"
#include "Skin.h"
#include "SkinFrameCtrl.h"


UIOBJECT_CLASS_NAME_IMP(CSkinFrameCtrl, "Frame")

CSkinFrameCtrl::CSkinFrameCtrl() {
    m_nRoundWidthTop = m_nRoundWidthBottom = 0;
    m_xBorder = m_yBorder = 0;
    m_nThickLeft = m_nThickTop = m_nThickRight = m_nThickBottom = 0;
    m_pObjFocusIndicator = nullptr;
    m_bpm = BPM_BLEND;
}

CSkinFrameCtrl::~CSkinFrameCtrl() {
}

void CSkinFrameCtrl::onCreate() {
    CUIObject::onCreate();

    m_font.setParent(m_pSkin);
}

void CSkinFrameCtrl::onAdjustHue(float hue, float saturation, float luminance) {
    m_font.onAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinFrameCtrl::draw(CRawGraph *canvas) {
    int xTextStart = 10;
    const int xTextBorder = 10;
    CSize sizeText = {0, 0};
    CRect rcFrame = m_rcObj;

    rcFrame.deflate(m_xBorder, 0);
    rcFrame.top += m_yBorder;

    if (m_strText.size() > 0) {
        canvas->getTextExtentPoint32(m_strText.c_str(), m_strText.size(), &sizeText);
        if (sizeText.cx > 0) {
            sizeText.cx += xTextBorder * 2;
        }
    } else {
        sizeText.cx = 0;
    }

    if (m_nRoundWidthTop > xTextStart) {
        xTextStart = m_nRoundWidthTop + rcFrame.left;
    } else {
        xTextStart += rcFrame.left;
    }

    CSFImage *pImage = &m_image;
    if (m_pObjFocusIndicator) {
        if (!m_pObjFocusIndicator->isEnable()) {
            pImage = &m_imageDisabled;
        } else if (m_imageFocus.isValid() && m_pSkin->getFocusUIObj() == m_pObjFocusIndicator) {
            pImage = &m_imageFocus;
        }
    } else if (m_imageFocus.isValid() && m_pSkin->isWndActive()) {
        pImage = &m_imageFocus;
    }

    // draw top left corner.
    pImage->blt(canvas, rcFrame.left, rcFrame.top, m_nRoundWidthTop, m_nRoundWidthTop, pImage->m_x, pImage->m_y, m_bpm);

    if (sizeText.cx > 0) {
        // draw top border - left
        pImage->xTileBlt(canvas, rcFrame.left + m_nRoundWidthTop, rcFrame.top,
            xTextStart - rcFrame.left, m_nThickTop, pImage->m_x + m_nRoundWidthTop, pImage->m_y,
            pImage->m_cx - m_nRoundWidthTop * 2, m_nThickTop, m_bpm);


        if (m_font.isGood()) {
            canvas->setFont(m_font.getFont());
        }

        // draw text
        CRect    rcText(xTextStart, m_rcObj.top,
            xTextStart + sizeText.cx, m_rcObj.top + m_yBorder * 2 + m_nThickTop);
        canvas->setTextColor(m_font.getTextColor(m_enable));
        canvas->drawText(m_strText.c_str(), m_strText.size(), rcText, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

        // draw top border right
        pImage->xTileBlt(canvas, rcText.right, rcFrame.top,
            rcFrame.right - rcText.right - m_nRoundWidthTop, m_nThickTop, pImage->m_x + m_nRoundWidthTop, pImage->m_y,
            pImage->m_cx - m_nRoundWidthTop * 2, m_nThickTop, m_bpm);
    } else {
        // draw top border
        pImage->xTileBlt(canvas, rcFrame.left + m_nRoundWidthTop, rcFrame.top,
            rcFrame.width() - m_nRoundWidthTop * 2, m_nThickTop, pImage->m_x + m_nRoundWidthTop, pImage->m_y,
            pImage->m_cx - m_nRoundWidthTop * 2, m_nThickTop, m_bpm);
    }

    // draw top right corner
    pImage->blt(canvas, rcFrame.right - m_nRoundWidthTop, rcFrame.top,
        m_nRoundWidthTop, m_nRoundWidthTop, pImage->m_x + pImage->m_cx - m_nRoundWidthTop, pImage->m_y, m_bpm);

    // draw left border
    pImage->yTileBlt(canvas, rcFrame.left, rcFrame.top + m_nRoundWidthTop,
        m_nThickLeft, rcFrame.height() - m_nRoundWidthTop - m_nRoundWidthBottom,
        pImage->m_x, pImage->m_y + m_nRoundWidthTop, m_nThickLeft, pImage->m_cy - m_nRoundWidthTop - m_nRoundWidthBottom, m_bpm);

    // draw right border
    pImage->yTileBlt(canvas, rcFrame.right - m_nThickRight, rcFrame.top + m_nRoundWidthTop,
        m_nThickRight, rcFrame.height() - m_nRoundWidthTop - m_nRoundWidthBottom,
        pImage->m_x + pImage->m_cx - m_nThickRight, pImage->m_y + m_nRoundWidthTop, m_nThickRight, pImage->m_cy - m_nRoundWidthTop - m_nRoundWidthBottom, m_bpm);

    // draw left bottom corner.
    pImage->blt(canvas, rcFrame.left, rcFrame.bottom - m_nRoundWidthBottom,
        m_nRoundWidthBottom, m_nRoundWidthBottom, pImage->m_x, pImage->m_y + pImage->m_cy - m_nRoundWidthBottom, m_bpm);

    // draw bottom border
    pImage->xTileBlt(canvas, rcFrame.left + m_nRoundWidthBottom, rcFrame.bottom - m_nThickBottom,
        rcFrame.width() - m_nRoundWidthBottom * 2, m_nThickBottom, pImage->m_x + m_nRoundWidthBottom, pImage->m_y + pImage->m_cy - m_nThickBottom,
        pImage->m_cx - m_nRoundWidthBottom * 2, m_nThickBottom, m_bpm);

    // draw right bottom corner
    pImage->blt(canvas, rcFrame.right - m_nRoundWidthBottom, rcFrame.bottom - m_nRoundWidthBottom,
        m_nRoundWidthBottom, m_nRoundWidthBottom,
        pImage->m_x + pImage->m_cx - m_nRoundWidthBottom, pImage->m_y + pImage->m_cy - m_nRoundWidthBottom, m_bpm);
}

bool CSkinFrameCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, SZ_PN_IMAGE)) {
        m_image.loadFromSRM(m_pSkin, szValue);
    } else if (isPropertyName(szProperty, SZ_PN_IMAGE_SIZE)) {
        scan2IntX(szValue, m_image.m_cx, m_image.m_cy);
    } else if (isPropertyName(szProperty, SZ_PN_IMAGE_POS)) {
        scan2IntX(szValue, m_image.m_x, m_image.m_y);
    } else if (isPropertyName(szProperty, SZ_PN_IMAGERECT)) {
        getRectValue(szValue, m_image);
    } else if (isPropertyName(szProperty, "ImageFocusPos")) {
        m_imageFocus = m_image;
        scan2IntX(szValue, m_imageFocus.m_x, m_imageFocus.m_y);
    } else if (isPropertyName(szProperty, "ImageDisabled")) {
        m_imageDisabled = m_image;
        scan2IntX(szValue, m_imageDisabled.m_x, m_imageDisabled.m_y);
    } else if (isPropertyName(szProperty, "BlendPixMode")) {
        m_bpm = blendPixModeFromStr(szValue);
    } else if (isPropertyName(szProperty, "RoundWidth")) {
        m_nRoundWidthTop = m_nRoundWidthBottom = atoi(szValue);
    } else if (isPropertyName(szProperty, "RoundWidthTop")) {
        m_nRoundWidthTop = atoi(szValue);
    } else if (isPropertyName(szProperty, "RoundWidthBottom")) {
        m_nRoundWidthBottom = atoi(szValue);
    } else if (isPropertyName(szProperty, "ThickWidth")) {
        if (!scan4IntX(szValue, m_nThickLeft, m_nThickTop, m_nThickRight, m_nThickBottom)) {
            if (scan2IntX(szValue, m_nThickLeft, m_nThickTop)) {
                m_nThickRight = m_nThickLeft;
                m_nThickBottom = m_nThickTop;
            } else {
                m_nThickLeft = m_nThickTop = m_nThickRight = m_nThickBottom = atoi(szValue);
            }
        }
    } else if (isPropertyName(szProperty, "XBorder")) {
        m_xBorder = atoi(szValue);
    } else if (isPropertyName(szProperty, "YBorder")) {
        m_yBorder = atoi(szValue);
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else {
        return false;
    }

    return true;
}
