// SkinFontProperty.cpp: implementation of the CSkinFontProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinFontProperty.h"


// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool fontPropertyValueFromStr(cstr_t szValue, int &nHeight, int &nWeight, uint8_t &byItalic, string &strFaceNameLatin9, string &strFaceNameOthers)
{
    string            strWeight;

    CCommaSeparatedValues csv;
    VecStrings    vValues;

    csv.split(szValue, vValues);
    trimStr(vValues);
    if (vValues.size() != 6)
        return false;

    strFaceNameLatin9 = vValues[0];
    nHeight = stringToInt(vValues[1].c_str(), -1);

    strWeight = vValues[2];
    if (strWeight.empty())
        nWeight = 13;
    else
    {
        if (strcasecmp(strWeight.c_str(), "bold") == 0)
            nWeight = FW_BOLD;
        else if (strcasecmp(strWeight.c_str(), "thin") == 0)
            nWeight = FW_THIN;
        else if (strcasecmp(strWeight.c_str(), "normal") == 0)
            nWeight = FW_NORMAL;
        else
            nWeight = atoi(strWeight.c_str());
    }

    byItalic = stringToInt(vValues[3].c_str(), 0);
    // int nUnderLine = stringToInt(vValues[4].c_str(), 0);
    strFaceNameOthers = vValues[5];

    return true;
}

void fontPropertyValueToStr(string &strValue, int nHeight, int nWeight, uint8_t byItalic, cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers)
{
#define            SZ_FEILD_SEP    ","
    strValue.resize(0);
    strValue += szFaceNameLatin9;
    strValue += SZ_FEILD_SEP;

    strValue += itos(nHeight); strValue += SZ_FEILD_SEP;

    strValue += itos(nWeight); strValue += SZ_FEILD_SEP;
/*    if (nWeight >= FW_BOLD)
        strValue += "bold, ";
    else if (nWeight <= FW_THIN)
        strValue += "thin, ";
    else
        strValue += "normal, ";*/

    strValue += itos(byItalic); strValue += SZ_FEILD_SEP;

    strValue += "0, ";

    strValue += szFaceNameOthers;
}


//////////////////////////////////////////////////////////////////////////

CSkinFontProperty::CSkinFontProperty()
{
    m_pRawFont = nullptr;
    m_pSkin = nullptr;
    m_nFlagProperties = 0;
    m_bCustomized = true;

    m_bOutlineText = false;
    m_clrText.set(RGB(0, 0, 0));
    m_clrTextOrg.set(RGB(0, 0, 0));
    m_clrTextDisbled.set(RGB(128, 128, 128));
    m_clrTextDisbledOrg.set(RGB(128, 128, 128));
    m_clrOutlined.set(RGB(192, 192, 192));
    m_clrOutlinedOrg.set(RGB(192, 192, 192));
}


CSkinFontProperty::~CSkinFontProperty()
{
    if (m_pRawFont)
        delete m_pRawFont;
}


void CSkinFontProperty::create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
    clear();

    m_font.create(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);
    m_nFlagProperties |= FP_FONT;
    m_bCustomized = true;
}


void CSkinFontProperty::create(const CFontInfo &font)
{
    clear();

    m_font.create(font);
    m_nFlagProperties |= FP_FONT;
    m_bCustomized = true;
}


void CSkinFontProperty::clear()
{
    if (m_pRawFont)
    {
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }

    m_nFlagProperties = 0;
    m_bCustomized = true;
    m_font.m_strNameLatin9.resize(0);
    m_font.m_strNameOthers.resize(0);
}


bool CSkinFontProperty::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (isPropertyName(szProperty, "CustomizedFont"))
        m_bCustomized = isTRUE(szValue);
    else if (isPropertyName(szProperty, "Font"))
    {
        // set font property in one field: Verdana, 13, thin, 1, 0, Tahoma
        fontPropertyValueFromStr(szValue, m_font.m_nSize, m_font.m_weight, m_font.m_nItalic, m_font.m_strNameLatin9, m_font.m_strNameOthers);
        m_nFlagProperties |= FP_FONT;
    }
    else if (isPropertyName(szProperty, "FontName"))
    {
        m_font.m_strNameLatin9 = szValue;
        if (!m_font.m_strNameLatin9.empty())
            m_nFlagProperties |= FP_NAME_LATIN9;
        else
            m_nFlagProperties &= ~FP_NAME_LATIN9;
    }
    else if (isPropertyName(szProperty, "FontNameOthers"))
    {
        m_font.m_strNameOthers = szValue;
        if (!m_font.m_strNameOthers.empty())
            m_nFlagProperties |= FP_NAME_OTHERS;
        else
            m_nFlagProperties &= ~FP_NAME_OTHERS;
    }
    else if (isPropertyName(szProperty, "FontHeight"))
    {
        m_font.m_nSize = atoi(szValue);
        if (m_font.m_nSize >= 8 && m_font.m_nSize < 255)
            m_nFlagProperties |= FP_HEIGHT;
        else
            m_nFlagProperties &= ~FP_HEIGHT;
    }
    else if (isPropertyName(szProperty, "FontBold"))
    {
        m_font.m_weight = isTRUE(szValue) ? FW_BOLD : FW_NORMAL;
        m_nFlagProperties |= FP_WEIGHT;
    }
    else if (isPropertyName(szProperty, "FontWeight"))
    {
        if (isPropertyName(szValue, "bold") == 0)
            m_font.m_weight = FW_BOLD;
        else if (isPropertyName(szValue, "thin") == 0)
            m_font.m_weight = FW_THIN;
        else
            m_font.m_weight = FW_NORMAL;
        m_nFlagProperties |= FP_WEIGHT;
    }
    else if (isPropertyName(szProperty, "FontItalic"))
    {
        m_font.m_nItalic = atoi(szValue);
        m_nFlagProperties |= FP_ITALIC;
    }
    else if (isPropertyName(szProperty, "FontUnderLine"))
    {
        m_font.m_nItalic = atoi(szValue);
        m_nFlagProperties |= FP_UNDERLINE;
    }
    else if (isPropertyName(szProperty, "TextOutlined"))
    {
        m_bOutlineText = isTRUE(szValue);
        m_nFlagProperties |= FP_OUTLINED;
    }
    else if (strcasecmp(szProperty, "TextColor") == 0)
    {
        getColorValue(m_clrTextOrg, szValue);
        m_clrText = m_clrTextOrg;
        m_nFlagProperties |= FP_TEXT_CLR;
    }
    else if (strcasecmp(szProperty, "DisabledTextColor") == 0)
    {
        getColorValue(m_clrTextDisbledOrg, szValue);
        m_clrTextDisbled = m_clrTextDisbledOrg;
        m_nFlagProperties |= FP_DISABLED_TEXT_CLR;
    }
    else if (strcasecmp(szProperty, "TextOutlinedColor") == 0)
    {
        getColorValue(m_clrOutlinedOrg, szValue);
        m_clrOutlined = m_clrOutlinedOrg;
        m_nFlagProperties |= FP_OUTLINED_CLR;
    }
    else
        return false;

    return true;
}


#ifdef _SKIN_EDITOR_
void CSkinFontProperty::enumProperties(CUIObjProperties &listProperties)
{
    listProperties.addPropBoolStr("CustomizedFont", m_bCustomized, !m_bCustomized);
    listProperties.addPropFontName("FontName", m_font.getName(), isFlagSet(m_nFlagProperties, FP_NAME_LATIN9));
    listProperties.addPropFontName("FontNameOthers", m_font.getNameOthers(), isFlagSet(m_nFlagProperties, FP_NAME_OTHERS));
    listProperties.addPropInt("FontHeight", m_font.getSize(), isFlagSet(m_nFlagProperties, FP_HEIGHT));
    listProperties.addPropInt("FontWeight", m_font.getWeight(), isFlagSet(m_nFlagProperties, FP_WEIGHT));
    listProperties.addPropInt("FontItalic", m_font.getItalic(), isFlagSet(m_nFlagProperties, FP_ITALIC));
    listProperties.addPropInt("FontUnderLine", m_font.IsUnderline(), isFlagSet(m_nFlagProperties, FP_UNDERLINE));
}
#endif // _SKIN_EDITOR_


cstr_t CSkinFontProperty::getName()
{
    if (isPropertySet(FP_NAME_LATIN9))
        return m_font.getName();
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->m_font.getName();
    else
        return "";
}


cstr_t CSkinFontProperty::getNameOthers()
{
    if (isPropertySet(FP_NAME_OTHERS))
        return m_font.getNameOthers();
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->m_font.getNameOthers();
    else
        return "";
}


int CSkinFontProperty::getSize()
{
    if (isPropertySet(FP_HEIGHT))
        return m_font.getSize();
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->m_font.getSize();
    else
        return 13;
}


int CSkinFontProperty::getWeight()
{
    if (isPropertySet(FP_WEIGHT))
        return m_font.getWeight();
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->m_font.getWeight();
    else
        return FW_NORMAL;
}


bool CSkinFontProperty::isOutlined()
{
    if (isPropertySet(FP_OUTLINED))
        return m_bOutlineText;
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->isOutlined();
    else
        return false;
}

int CSkinFontProperty::getHeight()
{
    CRawBmpFont *font = getFont();
    if (font == nullptr)
        return getSize();
    
    return font->getHeight();
}

const CColor &CSkinFontProperty::getTextColor(bool bEnabledClr)
{
    if (bEnabledClr)
    {
        if (isPropertySet(FP_TEXT_CLR))
            return m_clrText;
        else if (m_pSkin)
            return m_pSkin->getFontProperty()->getTextColor(bEnabledClr);
        else
            return m_clrText;
    }

    if (isPropertySet(FP_DISABLED_TEXT_CLR))
        return m_clrTextDisbled;
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->getTextColor(bEnabledClr);
    else
        return m_clrTextDisbled;
}


const CColor &CSkinFontProperty::getColorOutlined()
{
    if (isPropertySet(FP_OUTLINED_CLR))
        return m_clrOutlined;
    else if (m_pSkin)
        return m_pSkin->getFontProperty()->getColorOutlined();
    else
        return m_clrOutlined;
}


void CSkinFontProperty::onCreate(CSkinWnd *pSkin)
{
    m_pSkin = pSkin;

    if (m_pSkin)
    {
        m_pSkin->getSkinFactory()->getAdjustedHueResult(m_clrText);
        m_pSkin->getSkinFactory()->getAdjustedHueResult(m_clrTextDisbled);
        m_pSkin->getSkinFactory()->getAdjustedHueResult(m_clrOutlined);
    }
}


void CSkinFontProperty::onSkinFontChanged()
{
    assert(m_pSkin);

    if ((m_nFlagProperties & FP_FONT) == 0 || !m_bCustomized || !m_pSkin)
        return;

    updateMLFontInfo();

    if (!m_pRawFont)
        m_pRawFont = new CRawBmpFont;
    assert(m_pRawFont);

    if (!m_pRawFont->create(m_font))
    {
        delete m_pRawFont;
        m_pRawFont = nullptr;
    }
}


void CSkinFontProperty::onAdjustHue(CSkinWnd *pSkinwnd, float hue, float saturation, float luminance)
{
    if (isPropertySet(FP_TEXT_CLR))
    {
        m_clrText = m_clrTextOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrText);
    }

    if (isPropertySet(FP_DISABLED_TEXT_CLR))
    {
        m_clrTextDisbled = m_clrTextDisbledOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrTextDisbled);
    }

    if (isPropertySet(FP_OUTLINED_CLR))
    {
        m_clrOutlined = m_clrOutlinedOrg;
        pSkinwnd->getSkinFactory()->getAdjustedHueResult(m_clrOutlined);
    }
}


void CSkinFontProperty::updateMLFontInfo()
{
    assert(m_pSkin);
    if (!m_pSkin)
        return;

    CSkinFontProperty        *pFont;

    pFont = m_pSkin->getFontProperty();

    if (!isFlagSet(m_nFlagProperties, FP_NAME_LATIN9))
        m_font.m_strNameLatin9 = pFont->m_font.m_strNameLatin9;

    if (!isFlagSet(m_nFlagProperties, FP_NAME_OTHERS))
        m_font.m_strNameOthers = pFont->m_font.m_strNameOthers;

    if (!isFlagSet(m_nFlagProperties, FP_HEIGHT))
        m_font.m_nSize = pFont->m_font.m_nSize;

    if (!isFlagSet(m_nFlagProperties, FP_WEIGHT))
        m_font.m_weight = pFont->m_font.m_weight;

    if (!isFlagSet(m_nFlagProperties, FP_ITALIC))
        m_font.m_nItalic = pFont->m_font.m_nItalic;

    if (!isFlagSet(m_nFlagProperties, FP_UNDERLINE))
        m_font.m_bUnderline = pFont->m_font.m_bUnderline;
}


CRawBmpFont *CSkinFontProperty::getFont()
{
    if (m_bCustomized && (m_nFlagProperties & FP_FONT) != 0)
    {
        if (!m_pRawFont && m_pSkin)
            onSkinFontChanged();

        if (!m_pRawFont)
        {
            m_pRawFont = new CRawBmpFont;
            if (!m_pRawFont)
                return nullptr;

            m_pRawFont->create(m_font);
        }

        return m_pRawFont;
    }

    if (m_pSkin)
        return m_pSkin->getRawFont();
    else
        return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CSkinFontProperty)

class CTestCaseCSkinFontProperty : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCSkinFontProperty);
    CPPUNIT_TEST(testFontPropertyValueFromStr);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testFontPropertyValueFromStr()
    {
        int nHeight, nWeight;
        uint8_t byItalic;
        string strFaceNameLatin9, strFaceNameOthers;

        cstr_t szValue = "Verdana, 13, thin, 1, 0, Tahoma";

        CPPUNIT_ASSERT(fontPropertyValueFromStr(szValue, nHeight, nWeight, byItalic, strFaceNameLatin9, strFaceNameOthers));
        CPPUNIT_ASSERT(nHeight == 13);
        CPPUNIT_ASSERT(nWeight == FW_THIN);
        CPPUNIT_ASSERT(byItalic == 1);
        CPPUNIT_ASSERT(strFaceNameLatin9 == "Verdana");
        CPPUNIT_ASSERT(strFaceNameOthers == "Tahoma");
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCSkinFontProperty);

#endif // _CPPUNIT_TEST
