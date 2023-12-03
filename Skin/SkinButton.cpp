/********************************************************************
    Created  :    2001年12月15日 0:42:06
    FileName :    SkinButton.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinButton.h"


UIOBJECT_CLASS_NAME_IMP(CSkinButton, "Button")

CSkinButton::CSkinButton() {
    m_vBtStatImg.push_back(new BtStatImg);
    m_bRadioBt = false;
}

CSkinButton::~CSkinButton() {
}

bool CSkinButton::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinNStatusButton::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "CheckBox") == 0) {
        if (isTRUE(szValue)) {
            setMaxStat(2);
            m_msgNeed &= ~UO_MSG_WANT_ENTER_KEY;
        } else {
            setMaxStat(1);
        }
    } else if (strcasecmp(szProperty, "Radio") == 0) {
        m_bRadioBt = isTRUE(szValue);
        if (m_bRadioBt) {
            setMaxStat(2);
            m_msgNeed &= ~UO_MSG_WANT_ENTER_KEY;
        } else {
            setMaxStat(1);
        }
    } else if (strcasecmp(szProperty, "Checked") == 0) {
        m_nCurStatus = isTRUE(szValue) ? 1 : 0;
    } else if (strncasecmp(szProperty, SZ_PN_IMAGE, strlen(SZ_PN_IMAGE)) == 0) {
        string strNewProperty = "S0_";
        strNewProperty += szProperty;
        return CSkinNStatusButton::setProperty(strNewProperty.c_str(), szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinButton::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    if (m_vBtStatImg.empty()) {
        return;
    }

    listProperties.addPropBoolStr("CheckBox", m_vBtStatImg.size() == 2, m_vBtStatImg.size() == 2);
    listProperties.addPropBoolStr("Checked", m_nCurStatus == 1, m_vBtStatImg.size() == 2);

    {
        BtStatImg *statImg = m_vBtStatImg[0];

        listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, statImg->strBkFile.c_str(), statImg->imgBk);
        listProperties.addPropImage("ImageSel", "ImageSelRect", statImg->strSelFile.c_str(), statImg->imgSel, !statImg->strSelFile.empty());
        listProperties.addPropImage("ImageFocus", "ImageFocusRect", statImg->strHoverFile.c_str(), statImg->imgHover, !statImg->strHoverFile.empty());
    }

    if (m_vBtStatImg.size() > 1) {
        BtStatImg *statImg = m_vBtStatImg[1];

        listProperties.addPropImage("ImageCheck", "ImageCheckRect", statImg->strBkFile.c_str(), statImg->imgBk);
        listProperties.addPropImage("ImageCheckSel", "ImageCheckSelRect", statImg->strSelFile.c_str(), statImg->imgSel, !statImg->strSelFile.empty());
        listProperties.addPropImage("ImageCheckFocus", "ImageCheckFocusRect", statImg->strHoverFile.c_str(), statImg->imgHover, !statImg->strHoverFile.empty());
        listProperties.addPropImage("ImageDisable", "ImageDisableRect", statImg->strDisabledFile.c_str(), statImg->imgDisabled, !statImg->strDisabledFile.empty());
    }
}
#endif // _SKIN_EDITOR_

void CSkinButton::setCheck(bool bCheck) {
    assert(m_vBtStatImg.size() == 2);

    int nStatusNew = m_nCurStatus;
    if (m_vBtStatImg.size() == 2) {
        if (bCheck) {
            nStatusNew = 1;
        } else {
            nStatusNew = 0;
        }
    }
    if (nStatusNew != m_nCurStatus) {
        m_nCurStatus = nStatusNew;
        invalidate();
    }
}

bool CSkinButton::isCheck() {
    assert(m_vBtStatImg.size() == 2);

    return m_nCurStatus == 1;
}

void CSkinButton::buttonUpAction() {
    if (m_bRadioBt) {
        if (m_nCurStatus >= (int)m_vBtStatImg.size() - 1) {
            changeButtonState(BS_HOVER);
            return;
        }
    }

    CSkinNStatusButton::buttonUpAction();
}

UIOBJECT_CLASS_NAME_IMP(CSkinImageButton, "ImageButton")

CSkinImageButton::CSkinImageButton() {
}

CSkinImageButton::~CSkinImageButton() {

}

void CSkinImageButton::draw(CRawGraph *canvas) {
    CSkinButton::draw(canvas);

    CRect rc = m_rcObj;
    rc.deflate(m_nContentMarginX, m_nContentMarginY);
    if (m_bContentImage) {
        // DrawImage
        if (m_contentImage.isValid()) {
            if (m_isStretchContent) {
                m_contentImage.stretchBlt(canvas, rc.left, rc.top, rc.width(), rc.height());
            } else {
                m_contentImage.blt(canvas, rc, DT_CENTER | DT_VCENTER);
            }
        }
    } else {
        // Fill with color
        canvas->fillRect(rc, m_clrContent);
    }
}

bool CSkinImageButton::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinButton::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "ContentMarginX")) {
        m_nContentMarginX = atoi(szValue);
    } else if (isPropertyName(szProperty, "ContentMarginY")) {
        m_nContentMarginY = atoi(szValue);
    } else if (isPropertyName(szProperty, "ContentImage")) {
        m_bContentImage = true;
        m_contentImage.loadFromSRM(m_pSkin, szValue);
    } else if (isPropertyName(szProperty, "StretchContent")) {
        m_isStretchContent = isTRUE(szValue);
    } else {
        return false;
    }

    return true;
}

void CSkinImageButton::setContentImage(const RawImageDataPtr &image) {
    m_contentImage.attach(image);
    m_bContentImage = true;
}

void CSkinImageButton::setColor(const CColor &clr, bool bAutoSelColor) {
    m_clrContent = clr;
    m_bContentImage = false;
    m_bAutoSelColor = bAutoSelColor;
}

void CSkinImageButton::buttonUpAction() {
    if (!m_bContentImage && m_bAutoSelColor) {
        CDlgChooseColor dlg;

        if (dlg.doModal(m_pSkin, m_clrContent) == IDOK) {
            m_clrContent = dlg.getColor();
        } else {
            changeButtonState(BS_HOVER);
            return;
        }
    }

    return CSkinButton::buttonUpAction();
}
