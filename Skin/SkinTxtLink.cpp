#include "SkinTypes.h"
#include "Skin.h"
#include "SkinTxtLink.h"


UIOBJECT_CLASS_NAME_IMP(CSkinTxtLink, "TxtLink")

//////////////////////////////////////////////////////////////////////

CSkinTxtLink::CSkinTxtLink() {
    m_msgNeed = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE;
    m_dwAlignText = 0;
    m_bNeedResize = true;
}

CSkinTxtLink::~CSkinTxtLink() {
}

void CSkinTxtLink::draw(CRawGraph *canvas) {
    CRect rc;

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    if (m_bNeedResize) {
        m_bNeedResize = false;

        // resize control by content size.
        CSize size;
        if (canvas->getTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size)) {
            const int TEXT_MARGIN = 8;
            if (isFlagSet(m_dwAlignText, DT_RIGHT)) {
                m_rcObj.left = m_rcObj.right - size.cx - TEXT_MARGIN;
            } else if (isFlagSet(m_dwAlignText, DT_CENTER)) {
                m_rcObj.left = (m_rcObj.left + m_rcObj.right - size.cx - TEXT_MARGIN) / 2;
                m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
            } else {
                m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
            }

            if (isFlagSet(m_dwAlignText, DT_BOTTOM)) {
                m_rcObj.top = m_rcObj.bottom - size.cy - TEXT_MARGIN;
            } else if (isFlagSet(m_dwAlignText, DT_VCENTER)) {
                m_rcObj.top = (m_rcObj.top + m_rcObj.bottom - size.cy - 4) / 2;
                m_rcObj.bottom = m_rcObj.top + size.cy + 4;
            } else {
                m_rcObj.bottom = m_rcObj.top + size.cy + 4 / 2;
            }
        }
    }

    canvas->setTextColor(m_font.getTextColor(m_enable));

    rc = m_rcObj;
    rc.left += 4;
    rc.right -= 2;
    canvas->drawText(m_strText.c_str(), m_strText.size(), rc, m_dwAlignText | DT_SINGLELINE);
}

bool CSkinTxtLink::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_LINK) == 0) {
        m_strLink = szValue;
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else if (strcasecmp(szProperty, "AlignText") == 0) {
        m_dwAlignText = alignTextFromStr(szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinTxtLink::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropStr(SZ_PN_LINK, m_strLink.c_str());
    m_font.enumProperties(listProperties);
}
#endif // _SKIN_EDITOR_

void CSkinTxtLink::onCreate() {
    CUIObject::onCreate();

    m_font.onCreate(m_pSkin);
    m_font.setProperty("FontUnderLine", "1");

    m_Cursor.loadStdCursor(Cursor::C_HAND);
}

void CSkinTxtLink::onSize() {
    CUIObject::onSize();

    m_bNeedResize = true;
}

bool CSkinTxtLink::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (m_strLink.size()) {
        cstr_t SZ_CMD = "cmd://";
        if (startsWith(m_strLink.c_str(), SZ_CMD)) {
            int nId = m_pSkin->getSkinFactory()->getIDByName(m_strLink.c_str() + strlen(SZ_CMD));
            m_pSkin->postCustomCommandMsg(nId);
        } else {
            openUrl(m_pSkin, m_strLink.c_str());
        }
    }

    if (m_id != UID_INVALID) {
        m_pSkin->postCustomCommandMsg(m_id);
    }

    setCursor(m_Cursor);

    return true;
}

bool CSkinTxtLink::onLButtonDown(uint32_t nFlags, CPoint point) {
    setCursor(m_Cursor);

    return true;
}

bool CSkinTxtLink::onMouseMove(CPoint point) {
    setCursor(m_Cursor);

    return true;
}
