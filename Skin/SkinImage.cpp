/********************************************************************
    Created  :    2002/03/03    13:46
    FileName :    SkinImage.cpp
    Author   :    xhy

    Purpose  :    SKIN 中的可伸缩图片控件类
*********************************************************************/

#include "SkinTypes.h"
#include "Skin.h"


UIOBJECT_CLASS_NAME_IMP(CSkinNStatusImage, "NStatusImage")

CSkinNStatusImage::CSkinNStatusImage() {
    m_BltMode = BLT_COPY;
    m_nCurStatus = 0;
    m_bpm = BPM_BLEND;
    m_rcObj.setEmpty();
}

CSkinNStatusImage::~CSkinNStatusImage() {
    destroy();
}

void CSkinNStatusImage::onCreate() {
    CUIObject::onCreate();

    if (m_vStImages.size() > 0) {
        CSFImage &img = m_vStImages[0]->img;
        if (m_formWidth.empty()) {
            m_formWidth.setFormula(img.width());
        }
        if (m_formHeight.empty()) {
            m_formHeight.setFormula(img.height());
        }
    }
}

void CSkinNStatusImage::draw(CRawGraph *canvas) {
    if (m_vStImages.empty()) {
        return;
    }
    if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vStImages.size()) {
        m_nCurStatus = 0;
    }

    StatImg *si = m_vStImages[m_nCurStatus];

    if (m_BltMode == BLT_STRETCH) {
        si->img.stretchBlt(canvas, m_rcObj.left, m_rcObj.top,
            m_rcObj.width(), m_rcObj.height(), m_bpm);
    } else if (m_BltMode == BLT_TILE) {
        if (m_imgMask.isValid()) {
            si->img.tileMaskBlt(canvas, m_rcObj.left, m_rcObj.top,
                m_rcObj.width(), m_rcObj.height(), &m_imgMask, m_bpm);
        } else {
            si->img.tileBlt(canvas, m_rcObj.left, m_rcObj.top,
                m_rcObj.width(), m_rcObj.height(), m_bpm);
        }
    } else {
        if (m_imgMask.isValid()) {
            si->img.maskBlt(canvas, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
        } else {
            si->img.blt(canvas, m_rcObj.left, m_rcObj.top, m_bpm);
        }
    }
}

bool CSkinNStatusImage::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "CurStatus") == 0) {
        m_nCurStatus = atoi(szValue);
    } else if (szProperty[0] == 'S' && isDigit(szProperty[1]) && szProperty[2] == '_') {
        cstr_t szPropertyNew = szProperty + 3;
        int n = szProperty[1] - '0';
        if (n >= (int)m_vStImages.size()) {
            setMaxStat(n + 1);
        }
        StatImg *si = m_vStImages[n];

        if (strcasecmp(szPropertyNew, SZ_PN_IMAGE) == 0) {
            si->strImgFile = szValue;
            si->img.loadFromSRM(m_pSkin, szValue);
        } else if (strcasecmp(szPropertyNew, SZ_PN_IMAGERECT) == 0) {
            if (!getRectValue(szValue, si->img)) {
                ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
            }
        } else {
            return false;
        }
    } else if (strcasecmp(szProperty, "BltMode") == 0) {
        m_BltMode = bltModeFromStr(szValue);
    } else if (isPropertyName(szProperty, "BlendPixMode")) {
        m_bpm = blendPixModeFromStr(szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinNStatusImage::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropInt("CurStatus", m_nCurStatus);

    for (int i = 0; i < (int)m_vStImages.size(); i++) {
        StatImg *si = m_vStImages[i];
        string name, strNameRect;
        string strPrefix = "S0_";
        strPrefix[1] = '0' + i;

        name = strPrefix + SZ_PN_IMAGE;
        strNameRect = strPrefix + SZ_PN_IMAGERECT;
        listProperties.addPropImage(name.c_str(), strNameRect.c_str(), si->strImgFile.c_str(), si->img);
    }

    listProperties.addPropBltMode("BltMode", m_BltMode);
    listProperties.addPropBlendPixMode("BlendPixMode", m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_

void CSkinNStatusImage::setMaxStat(int nMaxStat) {
    if ((int)m_vStImages.size() > nMaxStat) {
        // remove extra stat
        for (int i = (int)m_vStImages.size() - 1; i >= nMaxStat; i--) {
            delete m_vStImages.back();
            m_vStImages.pop_back();
        }
    } else if ((int)m_vStImages.size() < nMaxStat) {
        // add extra stat
        for (int i = (int)m_vStImages.size(); i < nMaxStat; i++) {
            m_vStImages.push_back(new StatImg);
        }
    }
}

void CSkinNStatusImage::destroy() {
    for (int i = 0; i < (int)m_vStImages.size(); i++) {
        delete m_vStImages[i];
    }
    m_vStImages.clear();
}



UIOBJECT_CLASS_NAME_IMP(CSkinImage, "Image")

CSkinImage::CSkinImage() {
    m_BltMode = BLT_COPY;
}

CSkinImage::~CSkinImage() {
}

bool CSkinImage::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinNStatusImage::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0 || strcasecmp(szProperty, SZ_PN_IMAGERECT) == 0) {
        string strNewProperty = "S0_";
        strNewProperty += szProperty;
        return CSkinNStatusImage::setProperty(strNewProperty.c_str(), szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinImage::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    if (m_vStImages.size() >= 1) {
        StatImg *si = m_vStImages[0];
        listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, si->strImgFile.c_str(), si->img);
    } else {
        CSFImage img;
        listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, "", img);
    }

    listProperties.addPropBltMode("BltMode", m_BltMode);
}
#endif // _SKIN_EDITOR_

///////////////////////////////////////////////////////////////////////////////
// class CSkinActiveImage

UIOBJECT_CLASS_NAME_IMP(CSkinActiveImage, "ActiveImage")

CSkinActiveImage::CSkinActiveImage() {
}

CSkinActiveImage::~CSkinActiveImage() {
}

void CSkinActiveImage::draw(CRawGraph *canvas) {
    // 1 : active status
    if (m_pSkin->isWndActive()) {
        m_nCurStatus = 1;
    } else {
        m_nCurStatus = 0;
    }

    CSkinNStatusImage::draw(canvas);
}

bool CSkinActiveImage::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinImage::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "ImageFocus") == 0 ||
        strcasecmp(szProperty, "ImageActive") == 0) {
        string strNewProperty = "S1_";
        strNewProperty += SZ_PN_IMAGE;
        return CSkinNStatusImage::setProperty(strNewProperty.c_str(), szValue);
    } else if (strcasecmp(szProperty, "ImageFocusRect") == 0 ||
        strcasecmp(szProperty, "ImageActiveRect")) {
        string strNewProperty = "S1_";
        strNewProperty += SZ_PN_IMAGERECT;
        return CSkinNStatusImage::setProperty(strNewProperty.c_str(), szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinActiveImage::enumProperties(CUIObjProperties &listProperties) {
    CSkinImage::enumProperties(listProperties);

    if (m_vStImages.size() >= 2) {
        StatImg *si = m_vStImages[1];
        listProperties.addPropImage("ImageActive", "ImageActiveRect", si->strImgFile.c_str(), si->img);
    } else {
        CSFImage img;
        listProperties.addPropImage("ImageActive", "ImageActiveRect", "", img);
    }
}
#endif // _SKIN_EDITOR_
