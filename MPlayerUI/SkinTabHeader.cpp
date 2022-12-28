#include "Skin.h"
#include "SkinTabHeader.h"
#include "../Utils/Utils.h"


UIOBJECT_CLASS_NAME_IMP(CSkinTabHeader, "TabHeader")

CSkinTabHeader::CSkinTabHeader() {
    m_msgNeed = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON;

    m_bLBtDown = false;
    m_nActiveButton = 0;
    m_nHoverButton = -1;

    m_nButtonFaceSize = 0;
    m_nButtonBorderSize = 0;
    m_nTabHeight = 0;

    m_nActiveButtonIncSize = 0;

    m_bEnableHover = false;
    m_bHover = false;
}

CSkinTabHeader::~CSkinTabHeader() {
    g_profile.writeInt((string("TabActiveBt_") + m_strName).c_str(), m_nActiveButton);
}


CSkinTabHeader::Button::Button() {
    nID = UID_INVALID;
    nAutoUniID = UID_INVALID;
    nIDAssociatedObj = UID_INVALID;
    bTempToolTip = true;
    nImageStartPos = 0;
    nImageWidth = 0;
    nWidth = 0;
}


void CSkinTabHeader::Button::fromXML(CSkinTabHeader *pTabHeader, SXNode *pNode) {
    nAutoUniID = pTabHeader->m_pSkin->allocAutoUniID();

    SXNode::iterProperties it, itEnd;

    itEnd = pNode->listProperties.end();
    for (it = pNode->listProperties.begin(); it != itEnd; ++it) {
        SXNode::Property &prop = *it;
        cstr_t szProperty = prop.name.c_str();
        cstr_t szValue = prop.strValue.c_str();

        if (isPropertyName(szProperty, "width")) {
            nWidth = atoi(szValue);
        } else if (isPropertyName(szProperty, SZ_PN_ID)) {
            if (strTooltip.empty()) {
                nID = pTabHeader->m_pSkin->getSkinFactory()->getIDByNameEx(szValue, strTooltip);
                if (!strTooltip.empty()) {
                    bTempToolTip = true;
                }
            } else {
                nID = pTabHeader->m_pSkin->getSkinFactory()->getIDByName(szValue);
            }
        } else if (isPropertyName(szProperty, "ToolTip")) {
            if (strcmp(strTooltip.c_str(), szValue) != 0) {
                strTooltip = szValue;
                bTempToolTip = false;
            }
        } else if (isPropertyName(szProperty, "Text")) {
            strText = szValue;
        } else if (isPropertyName(szProperty, "IDAssociatedObj")) {
            strIDAssociatedObj = szValue;
            nIDAssociatedObj = pTabHeader->m_pSkin->getSkinFactory()->getIDByName(szValue);
        }
    }
}


#ifdef _SKIN_EDITOR_
void CSkinTabHeader::Button::toXML(CSkinTabHeader *pTabHeader, CXMLWriter &xml) {
    xml.writeStartElement("button");

    if (nID != UID_INVALID) {
        xml.writeAttribute(SZ_PN_ID, pTabHeader->m_pSkin->getSkinFactory()->getStringOfID(nID).c_str());
    }

    xml.writeAttribute("width", nWidth);
    if (!strTooltip.empty() && !bTempToolTip) {
        xml.writeAttribute("ToolTip", strTooltip.c_str());
    }
    if (!strText.empty()) {
        xml.writeAttribute("Text", strText.c_str());
    }
    if (!strIDAssociatedObj.empty()) {
        xml.writeAttribute("AssociatedObjID", strIDAssociatedObj.c_str());
    }

    xml.writeEndElement();
}
#endif // _SKIN_EDITOR_


void CSkinTabHeader::onCreate() {
    CUIObject::onCreate();

    m_font.onCreate(m_pSkin);

    m_nActiveButton = g_profile.getInt((string("TabActiveBt_") + m_strName).c_str(), m_nActiveButton);
    if (m_nActiveButton < 0 || m_nActiveButton >= (int)m_vButtons.size()) {
        m_nActiveButton = 0;
    }

    //
    // Hide/show all controls associated with TabHeader.
    //
    for (int i = 0; i < (int)m_vButtons.size(); i++) {
        Button &bt = m_vButtons[i];

        CUIObject *pObj = m_pSkin->getUIObjectById(bt.nIDAssociatedObj);
        if (pObj) {
            if (i == m_nActiveButton) {
                pObj->setVisible(true);
            } else {
                pObj->setVisible(false);
            }
        }
    }
}


bool CSkinTabHeader::onLButtonDown(uint32_t nFlags, CPoint point) {
    m_nHoverButton = buttonHitTest(point);
    if (m_nHoverButton == -1) {
        return false;
    }

    m_bLBtDown = true;

    if (m_bHover) {
        m_bHover = false;
    } else {
        // 捕捉鼠标输入
        m_pSkin->setCaptureMouse(this);
    }

    invalidate();

    return true;
}


bool CSkinTabHeader::onLButtonDblClk(uint32_t nFlags, CPoint point) {
    return false;
}

void CSkinTabHeader::onSize() {
    // The whole toolbar can't have the tooltip
    if (m_strTooltip.size()) {
        m_strTooltip.resize(0);
    }

    CUIObject::onSize();

    CRect rcBt;
    int i;

    // add tooltip of all the buttons.
    rcBt.left = m_rcObj.left;
    rcBt.top = m_rcObj.top;
    rcBt.bottom = m_rcObj.bottom;

    for (i = 0; i < (int)m_vButtons.size(); i++) {
        Button &bt = m_vButtons[i];
        if (bt.strTooltip.size()) {
            if (rcBt.left + bt.nWidth < m_rcObj.right) {
                rcBt.right = rcBt.left + bt.nWidth;
            } else {
                rcBt.right = m_rcObj.right;
            }

            m_pSkin->delTool(bt.nAutoUniID);
            m_pSkin->addTool(_TL(bt.strTooltip.c_str()), &rcBt, bt.nAutoUniID);
        }

        rcBt.left += bt.nWidth;
        if (rcBt.left >= m_rcObj.right) {
            break;
        }
    }
}

bool CSkinTabHeader::onLButtonUp(uint32_t nFlags, CPoint point) {
    int nNewCurButton = buttonHitTest(point);
    if (nNewCurButton == -1) {
        return false;
    }

    m_bLBtDown = false;

    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    // execute...
    if (nNewCurButton >= 0 && nNewCurButton < (int)m_vButtons.size()) {
        Button &bt = m_vButtons[nNewCurButton];

        if (bt.nID != UID_INVALID) {
            m_pSkin->postCustomCommandMsg(bt.nID);
        }

        // Hide old tab, show this tab.
        if (m_nActiveButton >= 0 && m_nActiveButton < (int)m_vButtons.size()) {
            CUIObject *pObj = m_pSkin->getUIObjectById(m_vButtons[m_nActiveButton].nIDAssociatedObj);
            if (pObj) {
                pObj->setVisible(false);
            }
        }

        CUIObject *pObj = m_pSkin->getUIObjectById(m_vButtons[nNewCurButton].nIDAssociatedObj);
        if (pObj) {
            pObj->setVisible(true);
        }

        m_nActiveButton = nNewCurButton;

        m_pSkin->invalidateRect();
    }

    return true;
}


bool CSkinTabHeader::onMouseMove(uint32_t nFlags, CPoint point) {
    assert(m_pSkin);

    if (isPtIn(point)) {
        int nOldCurButton = m_nHoverButton;

        m_nHoverButton = buttonHitTest(point);

        if (!m_bLBtDown && (nFlags & MK_LBUTTON)) {
            m_bLBtDown = true;

            if (m_bHover) {
                m_bHover = false;
            } else {
                // 捕捉鼠标输入
                m_pSkin->setCaptureMouse(this);
            }

            invalidate();
        } else if (!m_bLBtDown && m_bEnableHover) {
            m_bHover = true;

            // 捕捉鼠标输入
            m_pSkin->setCaptureMouse(this);

            invalidate();
        } else if (m_nHoverButton != nOldCurButton) {
            invalidate();
        }
    } else {
        int nOldCurButton = m_nHoverButton;

        m_nHoverButton = buttonHitTest(point);

        if (m_bLBtDown) {
            m_bLBtDown = false;

            // 释放鼠标输入
            m_pSkin->releaseCaptureMouse(this);

            invalidate();
        } else if (m_bHover) {
            m_bHover = false;

            // 释放鼠标输入
            m_pSkin->releaseCaptureMouse(this);

            invalidate();
        } else if (m_nHoverButton != nOldCurButton) {
            invalidate();
        }
    }

    return true;
}


void CSkinTabHeader::draw(CRawGraph *canvas) {
    int x, xActiveTabButton = 0;
    CRect rcButton;

    x = m_rcObj.left;

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    canvas->setTextColor(m_font.getTextColor(m_enable));

    rcButton.top = m_rcObj.top;
    rcButton.bottom = m_rcObj.top + m_nTabHeight;

    //
    // draw every table button
    //
    for (int i = 0; i < (int)m_vButtons.size(); i++) {
        Button &bt = m_vButtons[i];

        if (x + bt.nWidth >= m_rcObj.right) {
            break;
        }

        if (i == m_nActiveButton) {
            xActiveTabButton = x;
        } else {
            rcButton.left = x;
            rcButton.right = x + bt.nWidth;
            drawTabButton(canvas, bt, rcButton, false);
        }

        x += bt.nWidth;
    }

    if (m_nActiveButton >= 0 && m_nActiveButton < (int)m_vButtons.size()) {
        rcButton.left = xActiveTabButton;
        rcButton.right = xActiveTabButton + m_vButtons[m_nActiveButton].nWidth;
        drawTabButton(canvas, m_vButtons[m_nActiveButton], rcButton, true);
    }
}

bool CSkinTabHeader::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, SZ_PN_IMAGE)) {
        m_strImageFile = szValue;
        m_img.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (isPropertyName(szProperty, SZ_PN_IMAGERECT)) {
        if (!getRectValue(szValue, m_img)) {
            ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
        }
    } else if (isPropertyName(szProperty, "ButtonFaceSize")) {
        m_nButtonFaceSize = atoi(szValue);
    } else if (isPropertyName(szProperty, "ButtonBorderSize")) {
        m_nButtonBorderSize = atoi(szValue);
    } else if (isPropertyName(szProperty, "TabHeight")) {
        m_nTabHeight = atoi(szValue);
    } else if (isPropertyName(szProperty, "ActiveButton")) {
        m_nActiveButton = atoi(szValue);
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinTabHeader::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropImage(SZ_PN_IMAGE, SZ_PN_IMAGERECT, m_strImageFile.c_str(), m_img);
    listProperties.addPropInt("ButtonFaceSize", m_nButtonFaceSize, true);
    listProperties.addPropInt("BorderSize", m_nButtonBorderSize, true);
    listProperties.addPropInt("TabHeight", m_nTabHeight, true);
    listProperties.addPropInt("ActiveButton", m_nActiveButton, m_nActiveButton != 0);
    m_font.enumProperties(listProperties);
}
#endif // _SKIN_EDITOR_


int CSkinTabHeader::fromXML(SXNode *pXmlNode) {
    int nRet;

    nRet = CUIObject::fromXML(pXmlNode);
    if (nRet != ERR_OK) {
        return nRet;
    }

    for (SXNode::iterator it = pXmlNode->listChildren.begin();
    it != pXmlNode->listChildren.end(); ++it)
        {
        SXNode *pChild = *it;

        if (strcasecmp(pChild->name.c_str(), "button") == 0) {
            Button bt;
            bt.fromXML(this, pChild);
            m_vButtons.push_back(bt);
        }
    }

    return ERR_OK;
}


#ifdef _SKIN_EDITOR_
void CSkinTabHeader::onToXMLChild(CXMLWriter &xmlStream) {
    for (int i = 0; i < (int)m_vButtons.size(); i++) {
        Button &bt = m_vButtons[i];

        bt.toXML(this, xmlStream);
    }
}
#endif // _SKIN_EDITOR_


void CSkinTabHeader::invalidateButton(int nButton) {
    invalidate();
}


int CSkinTabHeader::buttonHitTest(CPoint pt) {
    if (!m_rcObj.ptInRect(pt)) {
        return -1;
    }

    pt.x -= m_rcObj.left;
    pt.y -= m_rcObj.top;

    int x = 0;

    for (int i = 0; i < (int)m_vButtons.size(); i++) {
        Button &bt = m_vButtons[i];

        x += bt.nWidth;
        if (x > m_rcObj.width()) {
            return -1;
        }

        if (pt.x < x) {
            return i;
        }
    }

    return -1;
}

void tillFill(CRawGraph *canvas, CRect *rcFill, RawImageData *rawImage, CRect *rcMask, const CColor &clrFill) {
    CRect rc = *rcFill;
    int nWidthMask = rcMask->right - rcMask->left;

    rc.right = rc.left + nWidthMask;
    if (rc.right > rcFill->right) {
        rc.right = rcFill->right;
    }
    for (; rc.right <= rcFill->right; rc.left += nWidthMask, rc.right += nWidthMask) {
        canvas->fillRect(&rc, rawImage, rcMask, clrFill);
    }

    if (rc.right > rcFill->right) {
        rc.right = rcFill->right;
        canvas->fillRect(&rc, rawImage, rcMask, clrFill);
    }
}

void CSkinTabHeader::drawTabButton(CRawGraph *canvas, Button &bt, CRect &rcButton, bool bActiveTabButton) {
    CRect rcMask;
    int nYPosTabButton;
    int x = rcButton.left;
    int y = rcButton.top;

    assert(m_img.isValid());
    if (!m_img.isValid()) {
        return;
    }

    // draw active button bg
    if (bActiveTabButton) {
        CRect rc;

        nYPosTabButton = TIP_ACTIVE * m_nTabHeight;

        rcMask.left = 0;
        rcMask.top = TIP_ACTIVE_MASK * m_nTabHeight;
        rcMask.right = m_img.width();
        rcMask.bottom = rcMask.top + m_nTabHeight;

        CColor clrBg = m_pContainer->getBgColor();

        // Fill left border
        rc = rcButton;
        rc.right = rc.left + m_nButtonBorderSize;
        canvas->fillRect(&rc, m_img.getHandle(), &rcMask, clrBg);

        // Button face
        rc.left = rc.right;
        rc.right = rcButton.right;
        rcMask.left += m_nButtonBorderSize;
        rcMask.right -= m_nButtonBorderSize;
        tillFill(canvas, &rc, m_img.getHandle(), &rcMask, clrBg);

        // Fill right border
        rc.left = rc.right;
        rc.right = rc.left + m_nButtonBorderSize;
        rcMask.left = rcMask.right;
        rcMask.right += m_nButtonBorderSize;
        canvas->fillRect(&rc, m_img.getHandle(), &rcMask, clrBg);
    } else {
        nYPosTabButton = TIP_INACTIVE * m_nTabHeight;
    }

    // draw left border
    m_img.blt(canvas, x, y, m_nButtonBorderSize, m_nTabHeight, 0, nYPosTabButton, BPM_BLEND);

    // draw button face
    m_img.xTileBlt(canvas, x + m_nButtonBorderSize, y, bt.nWidth - m_nButtonBorderSize, m_nTabHeight,
        m_nButtonBorderSize, nYPosTabButton, m_nButtonFaceSize, m_nTabHeight, BPM_BLEND);

    // draw Right border
    m_img.blt(canvas, rcButton.right, y, m_nButtonBorderSize, m_nTabHeight,
        m_nButtonFaceSize + m_nButtonBorderSize, nYPosTabButton, BPM_BLEND);

    // draw Text
    if (!bt.strText.empty()) {
        CRect rcText = rcButton;
        rcText.left += m_nButtonBorderSize;
        if (m_font.isOutlined()) {
            canvas->drawTextOutlined(bt.strText.c_str(), (int)bt.strText.size(), rcText, m_font.getTextColor(m_enable), m_font.getColorOutlined(), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            canvas->setTextColor(m_font.getTextColor(m_enable));
            canvas->drawText(bt.strText.c_str(), (int)bt.strText.size(), rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
    }
}
