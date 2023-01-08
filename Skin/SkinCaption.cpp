#include "SkinTypes.h"
#include "Skin.h"
#include "SkinCaption.h"


UIOBJECT_CLASS_NAME_IMP(CSkinCaption, "CaptionImage")

CSkinCaption::CSkinCaption() {
    m_bEnableMaximize = false;
    m_bpm = BPM_BLEND;
    m_nCutPos1 = m_nCutPos2 = m_nCutPos3 = m_nCutPos4 = 0;
}

CSkinCaption::~CSkinCaption() {

}


void CSkinCaption::draw(CRawGraph *canvas) {
    int nCenter;
    int nW1, nW2;
    CSFImage *pImg;

    // 窗口为激活状态
    if (m_pSkin->isWndActive() && m_imgFocus.isValid()) {
        pImg = &m_imgFocus;
    } else {
        pImg = &m_imgBk;
    }

    if (!pImg->isValid()) {
        return;
    }

    nCenter = pImg->m_cx / 2;
    nW1 = m_rcObj.width() / 2 - (m_nCutPos1 - pImg->m_x) - (pImg->m_x + nCenter - m_nCutPos2);
    nW2 = m_rcObj.width() - (m_nCutPos1 - pImg->m_x) - nW1 - (m_nCutPos3 - m_nCutPos2) - (pImg->m_x + pImg->m_cx - m_nCutPos4);

    //////////////////////////////////////////////////////////////////////////
    // *****1____2***3____4*****
    //        w1       w2
    //////////////////////////////////////////////////////////////////////////

    //
    // 左边的
    int x = m_rcObj.left;
    pImg->blt(canvas, x, m_rcObj.top,
        m_nCutPos1 - pImg->m_x, pImg->m_cy,
        pImg->m_x, pImg->m_y, m_bpm
        );

    //
    // 左中，拉伸绘画
    x += m_nCutPos1 - pImg->m_x;
    pImg->stretchBlt(canvas, x, m_rcObj.top,
        nW1, m_rcObj.height(),
        m_nCutPos1, pImg->m_y,
        m_nCutPos2 - m_nCutPos1, pImg->m_cy, m_bpm
        );

    //
    // 中间
    x += nW1;
    pImg->blt(canvas, x, m_rcObj.top,
        m_nCutPos3 - m_nCutPos2, pImg->m_cy,
        m_nCutPos2, pImg->m_y, m_bpm
        );

    //
    // 右中，拉伸绘画
    x += m_nCutPos3 - m_nCutPos2;
    pImg->stretchBlt(canvas, x, m_rcObj.top,
        nW2, m_rcObj.height(),
        m_nCutPos3, pImg->m_y,
        m_nCutPos4 - m_nCutPos3, pImg->m_cy, m_bpm
        );

    //
    // 右边的
    x += nW2;
    pImg->blt(canvas, x, m_rcObj.top,
        pImg->m_x + pImg->m_cx - m_nCutPos4, pImg->m_cy,
        m_nCutPos4, pImg->m_y, m_bpm
        );
}

bool CSkinCaption::onLButtonDown(uint32_t nFlags, CPoint point) {
    return false;
}

bool CSkinCaption::onLButtonUp(uint32_t nFlags, CPoint point) {
    return false;
}

bool CSkinCaption::onLButtonDblClk(uint32_t nFlags, CPoint point) {
    if (m_pSkin->isZoomed()) {
        m_pSkin->restore();
    } else {
        m_pSkin->maximize();
    }

    return true;
}

bool CSkinCaption::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        m_strBmpBkFile = szValue;
        m_imgBk.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0) {
        if (!getRectValue(szValue, m_imgBk)) {
            goto PARSE_VALUE_FAILED;
        }
    } else if (strcasecmp(szProperty, "ImageFocus") == 0) {
        m_strBmpFocusFile = szValue;
        m_imgFocus.loadFromSRM(m_pSkin, szValue);
    } else if (strcasecmp(szProperty, "ImageFocusRect") == 0) {
        if (!getRectValue(szValue, m_imgFocus)) {
            goto PARSE_VALUE_FAILED;
        }
    } else if (strcasecmp(szProperty, "EnableDblClick") == 0) {
        m_bEnableMaximize = isTRUE(szValue);
        if (m_bEnableMaximize) {
            m_msgNeed = UO_MSG_WANT_LBUTTON;
        } else {
            m_msgNeed = 0;
        }
    } else if (strcasecmp(szProperty, "StretchStartX") == 0) {
        m_nCutPos1 = atoi(szValue);
    } else if (strcasecmp(szProperty, "StretchEndX") == 0) {
        m_nCutPos2 = atoi(szValue);
    } else if (strcasecmp(szProperty, "StretchStartX2") == 0) {
        m_nCutPos3 = atoi(szValue);
    } else if (strcasecmp(szProperty, "StretchEndX2") == 0) {
        m_nCutPos4 = atoi(szValue);
    } else if (strcasecmp(szProperty, "CutPos") == 0) {
        if (!getRectValue(szValue, m_nCutPos1, m_nCutPos2, m_nCutPos3, m_nCutPos4)) {
            goto PARSE_VALUE_FAILED;
        }
    } else if (isPropertyName(szProperty, "BlendPixMode")) {
        m_bpm = blendPixModeFromStr(szValue);
    } else {
        return false;
    }

    return true;

PARSE_VALUE_FAILED:
    ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinCaption::enumProperties(CUIObjProperties &listProperties) {
    CUIObjProperty property;

    CUIObject::enumProperties(listProperties);

    listProperties.addPropBoolStr("EnableDblClick", m_bEnableMaximize, m_bEnableMaximize);

    vector<string> vExtra;
    char szBuff[64];

    _itot_s(m_nCutPos1, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchStartX");
    vExtra.push_back(szBuff);

    _itot_s(m_nCutPos2, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchEndX");
    vExtra.push_back(szBuff);

    _itot_s(m_nCutPos3, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchStartX2");
    vExtra.push_back(szBuff);

    _itot_s(m_nCutPos4, szBuff, CountOf(szBuff), 10);
    vExtra.push_back("StretchEndX2");
    vExtra.push_back(szBuff);

    listProperties.addPropImageEx(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strBmpBkFile.c_str(), m_imgBk, vExtra);

    listProperties.addPropImage("ImageFocus", "ImageFocusRect", m_strBmpFocusFile.c_str(), m_imgFocus, !m_strBmpFocusFile.empty());

    listProperties.addPropBlendPixMode("BlendPixMode", m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_
