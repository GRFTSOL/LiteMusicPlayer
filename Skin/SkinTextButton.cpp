#include "SkinTypes.h"
#include "Skin.h"
#include "SkinTextButton.h"


UIOBJECT_CLASS_NAME_IMP(CSkinTextButton,  "TextButton")

CSkinTextButton::CSkinTextButton() {
    m_dwAlignTextFlags = AT_VCENTER | AT_CENTER;
    m_bResizeToContent = false;
    m_nTextLeftMargin = m_nTextRightMargin = 0;
    m_chMenuKey = 0;
}

CSkinTextButton::~CSkinTextButton() {

}

#define TEXT_MARGIN         4

void CSkinTextButton::draw(CRawGraph *canvas) {
    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    if (m_bResizeToContent) {
        // resize button by content size.

        m_rcObj.right = m_rcObj.left + m_nTextLeftMargin + m_nTextRightMargin;

        CSize size;
        if (m_strText.size() && canvas->getTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size)) {
            m_rcObj.right += size.cx + TEXT_MARGIN * 2;
        }

        if (m_imgContent.isValid()) {
            m_rcObj.right += m_imgContent.m_cx;
            if (m_strText.size()) {
                m_rcObj.right += TEXT_MARGIN;
            }
        }
    }

    CSkinButton::draw(canvas);

    CRect rc = m_rcObj;
    rc.left += m_nTextLeftMargin;
    rc.right -= m_nTextRightMargin;

    // draw content image
    if (m_imgContent.isValid()) {
        m_imgContent.blt(canvas, rc.left, m_rcObj.top + (m_rcObj.height() - m_imgContent.m_cy) / 2);
        rc.left += TEXT_MARGIN + m_imgContent.m_cx;
    }

    // draw button text
    if (m_strText.size()) {
        CColor clrText = m_font.getTextColor(m_enable);

        if (m_font.isOutlined()) {
            canvas->drawTextClipOutlined(m_strText.c_str(), (int)m_strText.size(), rc,
                clrText, m_font.getColorOutlined(), m_dwAlignTextFlags | DT_PREFIX_TEXT | DT_SINGLELINE | DT_VCENTER);
        } else {
            canvas->setTextColor(clrText);
            canvas->drawText(m_strText.c_str(), m_strText.size(), rc, m_dwAlignTextFlags | DT_PREFIX_TEXT | DT_SINGLELINE | DT_VCENTER);
        }
    }
}

void CSkinTextButton::onCreate() {
    m_font.setParent(m_pSkin);

    CSkinButton::onCreate();
}

bool CSkinTextButton::onMenuKey(uint32_t nChar, uint32_t nFlags) {
    if (toUpper(m_chMenuKey) == toUpper(nChar)) {
        buttonUpAction();
        return true;
    }

    return false;
}

bool CSkinTextButton::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinButton::setProperty(szProperty, szValue)) {
        if (isPropertyName(szProperty, "CheckBox")
            || isPropertyName(szProperty, "Radio")) {
            m_bResizeToContent = true;
        }

        return true;
    }

    if (isPropertyName(szProperty, "AlignText")) {
        m_dwAlignTextFlags = alignTextFromStr(szValue);
    } else if (isPropertyName(szProperty, "ResizeToContent")) {
        m_bResizeToContent = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "ImageContent")) {
        m_imgContent.loadFromSRM(m_pSkin, szValue);
    } else if (isPropertyName(szProperty, "TextLeftMargin")) {
        m_nTextLeftMargin = atoi(szValue);
    } else if (isPropertyName(szProperty, "TextRightMargin")) {
        m_nTextRightMargin = atoi(szValue);
    } else if (!m_font.setProperty(szProperty, szValue)) {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinTextButton::enumProperties(CUIObjProperties &listProperties) {
    CSkinButton::enumProperties(listProperties);

    string str;
    alignTextToStr(m_dwAlignTextFlags, str);
    listProperties.addPropStr("AlignText", str.c_str());
    listProperties.addPropInt("TextLeftMargin", m_nTextLeft);
    m_font.enumProperties(listProperties);
    listProperties.addPropStr("Text", m_strText.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinTextButton::setText(cstr_t szText) {
    CUIObject::setText(szText);

    m_chMenuKey = getMenuKey(m_strText.c_str());
    if (m_chMenuKey != 0) {
        m_msgNeed |= UO_MSG_WANT_MENU_KEY;
    } else {
        m_msgNeed &= ~UO_MSG_WANT_MENU_KEY;
    }
}

void CSkinTextButton::onLanguageChanged() {
    CSkinButton::onLanguageChanged();

    m_chMenuKey = getMenuKey(m_strText.c_str());
    if (m_chMenuKey != 0) {
        m_msgNeed |= UO_MSG_WANT_MENU_KEY;
    } else {
        m_msgNeed &= ~UO_MSG_WANT_MENU_KEY;
    }
}
