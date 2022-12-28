#include "Skin.h"
#include "SkinFilterCtrl.h"


UIOBJECT_CLASS_NAME_IMP(CSkinFilterCtrl, "FilterCtrl");


CSkinFilterCtrl::CSkinFilterCtrl() : m_fStartPercent(1.0) {
}


CSkinFilterCtrl::~CSkinFilterCtrl() {
}


bool CSkinFilterCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "StartPercent")) {
        m_fStartPercent = (float)atof(szValue);
        if (m_fStartPercent < 0.0001 || m_fStartPercent > 1.0) {
            m_fStartPercent = 1.0;
        }
        return true;
    }

    return false;
}

#ifdef _SKIN_EDITOR_
void CSkinFilterCtrl::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);
}
#endif // _SKIN_EDITOR_


void CSkinFilterCtrl::draw(CRawGraph *canvas) {
    const int PIX_R = PixPosition::PIX_R;
    const int PIX_G = PixPosition::PIX_G;
    const int PIX_B = PixPosition::PIX_B;
    // const int PIX_A = PixPosition::PIX_A;
    const int PIX_SIZE = PixPosition::PIX_SIZE;

    CRect rcClip;
    canvas->getClipBoundBox(rcClip);
    rcClip.intersect(rcClip, m_rcObj);
    if (rcClip.empty() || m_rcObj.height() == 0) {
        return;
    }

    uint8_t *pSrc, *pDst;
    RawImageData *pMemData = canvas->getRawBuff();
    uint16_t nByteStride = rcClip.width() * PIX_SIZE;
    int nByteLeftOffset = rcClip.left * PIX_SIZE;
    float fPercent = m_fStartPercent;
    float fStep = fPercent / m_rcObj.height();

    // Fade out and copy the graph above.
    for (int i = rcClip.top - m_rcObj.top;
         i < rcClip.bottom - m_rcObj.top
            && fPercent > 0.0000001
    && m_rcObj.top - i - 1 >= 0;
    ++i, fPercent -= fStep)
    {
        pSrc = pMemData->rowPtr(m_rcObj.top - i - 1) + nByteLeftOffset;
        pDst = pMemData->rowPtr(rcClip.top + i) + nByteLeftOffset;
        uint8_t * pEnd = pSrc + nByteStride;

        for (; pSrc < pEnd; pSrc += PIX_SIZE, pDst += PIX_SIZE) {
            // a * f + b * (1 - f) = (a - b) * f + b
            pDst[PIX_R] = (uint8_t)((pSrc[PIX_R] - pDst[PIX_R]) * fPercent + pDst[PIX_R]);
            pDst[PIX_G] = (uint8_t)((pSrc[PIX_G] - pDst[PIX_G]) * fPercent + pDst[PIX_G]);
            pDst[PIX_B] = (uint8_t)((pSrc[PIX_B] - pDst[PIX_B]) * fPercent + pDst[PIX_B]);
        }
    }
}
