#include "SkinTypes.h"
#include "Skin.h"
#include "SkinFontProperty.h"


//////////////////////////////////////////////////////////////////////////

CSkinFontProperty::CSkinFontProperty() {
    m_pRawFont = nullptr;
    m_parent = nullptr;
    m_nFlagProperties = 0;

    m_bOutlineText = false;
    m_clrText.set(RGB(0, 0, 0));
    m_clrTextOrg.set(RGB(0, 0, 0));
    m_clrTextDisbled.set(RGB(128, 128, 128));
    m_clrTextDisbledOrg.set(RGB(128, 128, 128));
    m_clrOutlined.set(RGB(192, 192, 192));
    m_clrOutlinedOrg.set(RGB(192, 192, 192));
}


CSkinFontProperty::~CSkinFontProperty() {
    if (m_pRawFont) {
        delete m_pRawFont;
    }
}


void CSkinFontProperty::create(const FontInfoEx &font) {
    clear();

    m_font = font;
    m_nFlagProperties |= FP_FONT;

    m_pRawFont = new CRawBmpFont;
    if (!m_pRawFont->create(m_font, 2)) {
        assert(0);
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }
}

void CSkinFontProperty::create() {
    if (m_pRawFont) {
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }

    m_pRawFont = new CRawBmpFont;
    if (!m_pRawFont->create(m_font, 2)) {
        assert(0);
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }
}

void CSkinFontProperty::clear() {
    if (m_pRawFont) {
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }

    m_nFlagProperties = 0;
    m_font.clear();
}

bool CSkinFontProperty::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (isPropertyName(szProperty, "Font")) {
        // set font property in one field: Verdana, 13, thin, 1, 0, Tahoma
        m_font.parse(szValue);
        m_nFlagProperties |= FP_FONT;
    } else if (isPropertyName(szProperty, "FontName")) {
        m_font.nameLatin9 = szValue;
        if (!m_font.nameLatin9.empty()) {
            m_nFlagProperties |= FP_NAME_LATIN9;
        } else {
            m_nFlagProperties &= ~FP_NAME_LATIN9;
        }
    } else if (isPropertyName(szProperty, "FontNameOthers")) {
        m_font.nameOthers = szValue;
        if (!m_font.nameOthers.empty()) {
            m_nFlagProperties |= FP_NAME_OTHERS;
        } else {
            m_nFlagProperties &= ~FP_NAME_OTHERS;
        }
    } else if (isPropertyName(szProperty, "FontHeight")) {
        m_font.height = atoi(szValue);
        if (m_font.height >= 8 && m_font.height < 255) {
            m_nFlagProperties |= FP_HEIGHT;
        } else {
            m_nFlagProperties &= ~FP_HEIGHT;
        }
    } else if (isPropertyName(szProperty, "FontBold")) {
        m_font.weight = isTRUE(szValue) ? FW_BOLD : FW_NORMAL;
        m_nFlagProperties |= FP_WEIGHT;
    } else if (isPropertyName(szProperty, "FontWeight")) {
        if (isPropertyName(szValue, "bold") == 0) {
            m_font.weight = FW_BOLD;
        } else if (isPropertyName(szValue, "thin") == 0) {
            m_font.weight = FW_THIN;
        } else {
            m_font.weight = FW_NORMAL;
        }
        m_nFlagProperties |= FP_WEIGHT;
    } else if (isPropertyName(szProperty, "FontItalic")) {
        m_font.italic = atoi(szValue);
        m_nFlagProperties |= FP_ITALIC;
    } else if (isPropertyName(szProperty, "FontUnderLine")) {
        m_font.isUnderline = atoi(szValue);
        m_nFlagProperties |= FP_UNDERLINE;
    } else if (isPropertyName(szProperty, "TextOutlined")) {
        m_bOutlineText = isTRUE(szValue);
        m_nFlagProperties |= FP_OUTLINED;
    } else if (strcasecmp(szProperty, "TextColor") == 0) {
        getColorValue(m_clrTextOrg, szValue);
        m_clrText = m_clrTextOrg;
        m_nFlagProperties |= FP_TEXT_CLR;
    } else if (strcasecmp(szProperty, "DisabledTextColor") == 0) {
        getColorValue(m_clrTextDisbledOrg, szValue);
        m_clrTextDisbled = m_clrTextDisbledOrg;
        m_nFlagProperties |= FP_DISABLED_TEXT_CLR;
    } else if (strcasecmp(szProperty, "TextOutlinedColor") == 0) {
        getColorValue(m_clrOutlinedOrg, szValue);
        m_clrOutlined = m_clrOutlinedOrg;
        m_nFlagProperties |= FP_OUTLINED_CLR;
    } else {
        return false;
    }

    return true;
}


#ifdef _SKIN_EDITOR_
void CSkinFontProperty::enumProperties(CUIObjProperties &listProperties) {
    listProperties.addPropFontName("FontName", m_font.getName(), isFlagSet(m_nFlagProperties, FP_NAME_LATIN9));
    listProperties.addPropFontName("FontNameOthers", m_font.getNameOthers(), isFlagSet(m_nFlagProperties, FP_NAME_OTHERS));
    listProperties.addPropInt("FontHeight", m_font.getSize(), isFlagSet(m_nFlagProperties, FP_HEIGHT));
    listProperties.addPropInt("FontWeight", m_font.getWeight(), isFlagSet(m_nFlagProperties, FP_WEIGHT));
    listProperties.addPropInt("FontItalic", m_font.getItalic(), isFlagSet(m_nFlagProperties, FP_ITALIC));
    listProperties.addPropInt("FontUnderLine", m_font.IsUnderline(), isFlagSet(m_nFlagProperties, FP_UNDERLINE));
}
#endif // _SKIN_EDITOR_


cstr_t CSkinFontProperty::getName() {
    if (isPropertySet(FP_NAME_LATIN9)) {
        return m_font.getName();
    } else if (m_parent) {
        return m_parent->getName();
    } else {
        return "";
    }
}


cstr_t CSkinFontProperty::getNameOthers() {
    if (isPropertySet(FP_NAME_OTHERS)) {
        return m_font.getNameOthers();
    } else if (m_parent) {
        return m_parent->getNameOthers();
    } else {
        return "";
    }
}


int CSkinFontProperty::getWeight() {
    if (isPropertySet(FP_WEIGHT)) {
        return m_font.getWeight();
    } else if (m_parent) {
        return m_parent->getWeight();
    } else {
        return FW_NORMAL;
    }
}


bool CSkinFontProperty::isOutlined() {
    if (isPropertySet(FP_OUTLINED)) {
        return m_bOutlineText;
    } else if (m_parent) {
        return m_parent->isOutlined();
    } else {
        return false;
    }
}

int CSkinFontProperty::getHeight() {
    if (isPropertySet(FP_HEIGHT)) {
        return m_font.height;
    } else if (m_parent) {
        return m_parent->getHeight();
    } else {
        return 13;
    }
}

const CColor &CSkinFontProperty::getTextColor(bool bEnabledClr) {
    if (bEnabledClr) {
        if (isPropertySet(FP_TEXT_CLR)) {
            return m_clrText;
        } else if (m_parent) {
            return m_parent->getTextColor(bEnabledClr);
        } else {
            return m_clrText;
        }
    }

    if (isPropertySet(FP_DISABLED_TEXT_CLR)) {
        return m_clrTextDisbled;
    } else if (m_parent) {
        return m_parent->getTextColor(bEnabledClr);
    } else {
        return m_clrTextDisbled;
    }
}


const CColor &CSkinFontProperty::getColorOutlined() {
    if (isPropertySet(FP_OUTLINED_CLR)) {
        return m_clrOutlined;
    } else if (m_parent) {
        return m_parent->getColorOutlined();
    } else {
        return m_clrOutlined;
    }
}

void CSkinFontProperty::setParent(CSkinWnd *skinWnd) {
    m_parent = skinWnd->getFontProperty();

    if (skinWnd) {
        skinWnd->getSkinFactory()->getAdjustedHueResult(m_clrText);
        skinWnd->getSkinFactory()->getAdjustedHueResult(m_clrTextDisbled);
        skinWnd->getSkinFactory()->getAdjustedHueResult(m_clrOutlined);
    }
}

void CSkinFontProperty::onSkinFontChanged() {
    assert(m_parent);

    if ((m_nFlagProperties & FP_FONT) == 0 || !m_parent) {
        return;
    }

    assert(m_parent);
    if (!m_parent) {
        return;
    }

    auto &font = m_parent->getFont()->getFontInfo();
    if (!isFlagSet(m_nFlagProperties, FP_NAME_LATIN9)) { m_font.nameLatin9 = font.nameLatin9; }
    if (!isFlagSet(m_nFlagProperties, FP_NAME_OTHERS)) { m_font.nameOthers = font.nameOthers; }
    if (!isFlagSet(m_nFlagProperties, FP_HEIGHT)) { m_font.height = font.height; }
    if (!isFlagSet(m_nFlagProperties, FP_WEIGHT)) { m_font.weight = font.weight; }
    if (!isFlagSet(m_nFlagProperties, FP_ITALIC)) { m_font.italic = font.italic; }
    if (!isFlagSet(m_nFlagProperties, FP_UNDERLINE)) { m_font.isUnderline = font.isUnderline; }

    if (!m_pRawFont) {
        m_pRawFont = new CRawBmpFont;
    }

    if (!m_pRawFont->create(m_font, 1)) {
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }
}

void CSkinFontProperty::onAdjustHue(CSkinWnd *pSkinwnd, float hue, float saturation, float luminance) {
    if (isPropertySet(FP_TEXT_CLR)) {
        m_clrText = m_clrTextOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrText);
    }

    if (isPropertySet(FP_DISABLED_TEXT_CLR)) {
        m_clrTextDisbled = m_clrTextDisbledOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrTextDisbled);
    }

    if (isPropertySet(FP_OUTLINED_CLR)) {
        m_clrOutlined = m_clrOutlinedOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrOutlined);
    }
}


CRawBmpFont *CSkinFontProperty::getFont() {
    if (m_nFlagProperties & FP_FONT) {
        if (!m_pRawFont) {
            onSkinFontChanged();
        }

        return m_pRawFont;
    }

    assert(m_parent);
    return m_parent->getFont();
}
