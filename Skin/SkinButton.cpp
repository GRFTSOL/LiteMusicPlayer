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
            m_bLBtDown = false;
            invalidate();
            return;
        }
    }

    CSkinNStatusButton::buttonUpAction();
}

///////////////////////////////////////////////////////////////////////////////
// class CSkinActiveButton
// set classname
UIOBJECT_CLASS_NAME_IMP(CSkinActiveButton, "ActiveButton")

CSkinActiveButton::CSkinActiveButton() {
}

CSkinActiveButton::~CSkinActiveButton() {

}

void CSkinActiveButton::draw(CRawGraph *canvas) {
    if (m_vBtStatImg.empty()) {
        return;
    }

    if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vBtStatImg.size()) {
        m_nCurStatus = 0;
    }

    BtStatImg *btimg = m_vBtStatImg[m_nCurStatus];

    if (m_enable && !m_bLBtDown && !m_bHover) {
        if (m_pSkin->isWndActive()) {
            if (m_imgMask.isValid()) {
                btimg->imgHover.maskBlt(canvas, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
            } else {
                btimg->imgHover.blt(canvas, m_rcObj.left, m_rcObj.top, m_bpm);
            }
            return;
        }
    }

    CSkinNStatusButton::draw(canvas);
}

// class CSkinActiveButton
///////////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinImageButton, "ImageButton")

CSkinImageButton::CSkinImageButton() {
    m_bContentImage = true;
    m_bAutoSelColor = false;
    m_nContentMarginX = 5;
    m_nContentMarginY = 5;
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
            m_contentImage.stretchBlt(canvas, rc.left, rc.top, rc.width(), rc.height());
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
            m_bLBtDown = false;
            invalidate();
            return;
        }
    }

    return CSkinButton::buttonUpAction();
}

///////////////////////////////////////////////////////////////////////////////
UIOBJECT_CLASS_NAME_IMP(CSkinHoverActionButton, "HoverActionButton")

CSkinHoverActionButton::CSkinHoverActionButton() {
    m_nTimerIdHoverAction = 0;
    m_bHoverActionBegin = false;
    m_timeBeginHover = 0;
}

CSkinHoverActionButton::~CSkinHoverActionButton() {
}

bool CSkinHoverActionButton::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (!CSkinButton::onLButtonUp(nFlags, point)) {
        return false;
    }

    if (!m_bHover) {
        m_bHover = true;

        // 捕捉鼠标输入
        m_pSkin->setCaptureMouse(this);
    }

    return true;
}

bool CSkinHoverActionButton::onMouseMove(CPoint point) {
    bool bHoverPrev = m_bHover;

    bool bRet = CSkinButton::onMouseMove(point);

    if (!m_bLBtDown && bHoverPrev != m_bHover) {
        // NOT in button down && hover status changed.
        if (m_bHover) {
            m_nTimerIdHoverAction = m_pSkin->registerTimerObject(this, 500);
            m_bHoverActionBegin = true;
        } else {
            endHover();
        }
    }

    return bRet;
}

void CSkinHoverActionButton::onTimer(int nId) {
    if (nId == m_nTimerIdHoverAction) {
        if (m_bHoverActionBegin) {
            m_bHoverActionBegin = false;
            m_pSkin->unregisterTimerObject(this, m_nTimerIdHoverAction);
            m_nTimerIdHoverAction = m_pSkin->registerTimerObject(this, 100);
            m_timeBeginHover = getTickCount();
            dispatchEvent(CSkinHoverActionBtEventNotify::C_BEGIN_HOVER);
        } else {
            auto now = getTickCount();
            const int MAX_HOVER_ACTION_TIME = 20 * 1000;
            if (now - m_timeBeginHover >= MAX_HOVER_ACTION_TIME) {
                endHover();
            } else {
                dispatchEvent(CSkinHoverActionBtEventNotify::C_HOVER_ACTION);
            }
        }

        return;
    }

    CSkinButton::onTimer(nId);
}

void CSkinHoverActionButton::dispatchEvent(CSkinHoverActionBtEventNotify::Command cmd) {
    CSkinHoverActionBtEventNotify event(this);

    event.cmd = cmd;
    m_pSkin->dispatchUIObjNotify(&event);
}

void CSkinHoverActionButton::endHover() {
    m_pSkin->unregisterTimerObject(this, m_nTimerIdHoverAction);
    m_nTimerIdHoverAction = 0;
    dispatchEvent(CSkinHoverActionBtEventNotify::C_END_HOVER);
}

void CSkinHoverActionButton::buttonUpAction() {
    dispatchEvent(CSkinHoverActionBtEventNotify::C_CLICK);

    return CSkinButton::buttonUpAction();
}
