/********************************************************************
    Created  :    2001年12月15日 0:42:06
    FileName :    SkinStaticText.cpp
    Author   :    xhy

    Purpose  :    
*********************************************************************/

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinStaticText.h"


UIOBJECT_CLASS_NAME_IMP(CSkinStaticText, "Text")

CSkinStaticText::CSkinStaticText() {
    m_nLeftMargin = 2;
    m_bCaptionText = false;

    m_dwAlignText = AT_LEFT;
}

CSkinStaticText::~CSkinStaticText() {

}

void CSkinStaticText::onCreate() {
    CUIObject::onCreate();

    m_font.setParent(m_pSkin);
}

int countOfChar(cstr_t str, char ch) {
    int count = 0;
    while (*str) {
        if (*str == ch) {
            count++;
        }
        str++;
    }
    return count;
}

void CSkinStaticText::onMeasureSizeByContent() {
    CRawGraph *canvas = m_pContainer->getMemGraph();

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    if (m_layoutParams & (LAYOUT_HEIGHT_WRAP_CONENT | LAYOUT_WIDTH_WRAP_CONENT)) {
        int nLines = countOfChar(m_strText.c_str(), '\n') + 1;
        const int TEXT_MARGIN = 8;
        CSize size;
        if (nLines > 1 && isFlagSet(m_layoutParams, LAYOUT_WIDTH_WRAP_CONENT)) {
            int widthMax = 0;
            VecStrings vLines;
            strSplit(m_strText.c_str(), '\n', vLines);
            for (uint32_t i = 0; i < vLines.size(); i++) {
                if (canvas->getTextExtentPoint32(vLines[i].c_str(), vLines[i].size(), &size)
                    && size.cx > widthMax) {
                    widthMax = size.cx;
                }
            }
        } else {
            canvas->getTextExtentPoint32(m_strText.c_str(), m_strText.size(), &size);
        }

        if (m_layoutParams & LAYOUT_HEIGHT_WRAP_CONENT) {
            m_rcObj.bottom = m_rcObj.top + (size.cy + 2) * nLines + TEXT_MARGIN / 2;
        }

        if (m_layoutParams & LAYOUT_WIDTH_WRAP_CONENT) {
            m_rcObj.right = m_rcObj.left + size.cx + TEXT_MARGIN;
        }
    }
}

void CSkinStaticText::onAdjustHue(float hue, float saturation, float luminance) {
    m_font.onAdjustHue(m_pSkin, hue, saturation, luminance);
}

bool CSkinStaticText::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (m_id != UID_INVALID) {
        m_pSkin->postCustomCommandMsg(m_id);
    }

    return true;
}

void CSkinStaticText::draw(CRawGraph *canvas) {
    CUIObject::draw(canvas);

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    CRect rc = m_rcObj;
    cstr_t szText = nullptr;

    rc.left += m_nLeftMargin;

    if (m_bCaptionText) {
        szText = m_pSkin->getCaptionText();
    } else {
        szText = m_strText.c_str();
    }

    CColor clrText = m_font.getTextColor(m_enable);

    if (m_font.isOutlined()) {
        canvas->drawTextOutlined(szText, (int)strlen(szText), rc,
            clrText, m_font.getColorOutlined(), m_dwAlignText | DT_END_ELLIPSIS | DT_PREFIX_TEXT);
    } else {
        canvas->setTextColor(clrText);
        canvas->drawText(szText, (int)strlen(szText), rc, m_dwAlignText | DT_END_ELLIPSIS | DT_PREFIX_TEXT);
    }
}

bool CSkinStaticText::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else if (strcasecmp(szProperty, "LeftMargin") == 0) {
        m_nLeftMargin = atoi(szValue);
    } else if (isPropertyName(szProperty, "CaptionText")) {
        m_bCaptionText = isTRUE(szValue);
    } else if (strcasecmp(szProperty, "AlignText") == 0) {
        m_dwAlignText = alignTextFromStr(szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinStaticText::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    m_font.enumProperties(listProperties);
    listProperties.addPropInt("LeftMargin", m_nLeftMargin);
    listProperties.addPropStr("Text", m_strText.c_str(), !m_strText.empty());

    string str;
    alignTextToStr(m_dwAlignText, str);
    listProperties.addPropStr("AlignText", str.c_str());
}
#endif // _SKIN_EDITOR_
