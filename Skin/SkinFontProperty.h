
#pragma once


class CUIObjProperties;

// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool fontPropertyValueFromStr(cstr_t szValue, int &nHeight, int &nWeight, uint8_t &byItalic, string &strFaceNameLatin9, string &strFaceNameOthers);
void fontPropertyValueToStr(string &strValue, int nHeight, int nWeight, uint8_t byItalic, cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers);


//
// This class is used to manage font properties of skin window and skin controls.
//
// Skin window has font property, every skin control(UIObject) inherits font property from it,
// but every control can use customized font name, size or weight etc, 
// And user may modify default font properties of skin winodw, 
// and the child controls should be affected, if it use default values.
//
class CSkinFontProperty
{
public:
    enum FontProperty
    {
        FP_NAME_LATIN9        = 0x1,
        FP_NAME_OTHERS        = 0x1 << 1,
        FP_HEIGHT            = 0x1 << 2,
        FP_WEIGHT            = 0x1 << 3,
        FP_ITALIC            = 0x1 << 4,
        FP_UNDERLINE        = 0x1 << 5,
        FP_OUTLINED            = 0x1 << 6,
        FP_TEXT_CLR            = 0x1 << 7,
        FP_DISABLED_TEXT_CLR= 0x1 << 8,
        FP_OUTLINED_CLR        = 0x1 << 9,

        FP_FONT = FP_NAME_LATIN9 | FP_NAME_OTHERS | FP_HEIGHT | FP_WEIGHT | FP_ITALIC | FP_UNDERLINE
    };

    CSkinFontProperty();
    virtual ~CSkinFontProperty();

    void create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline);

    void create(const CFontInfo &font);

    void clear();

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    cstr_t getName();
    cstr_t getNameOthers();
    int getSize();
    int getWeight();
    bool isOutlined();
    
    int getHeight();

    const CColor &getTextColor(bool bEnabledClr = true);
    const CColor &getColorOutlined();

    bool isGood() { return m_pSkin != nullptr || (m_bCustomized && isFlagSet(m_nFlagProperties, FP_FONT)); }
    CRawBmpFont *getFont();

    void updateMLFontInfo();

    CFontInfo & getMLFont() { return m_font; }

    void onCreate(CSkinWnd *pSkin);

    void onSkinFontChanged();

    void onAdjustHue(CSkinWnd *pSkinwnd, float hue, float saturation, float luminance);

protected:
    inline bool isPropertySet(uint32_t fontProperty) { return isFlagSet(m_nFlagProperties, fontProperty); }

protected:
    CSkinWnd            *m_pSkin;
    CRawBmpFont        *m_pRawFont;
    CFontInfo           m_font;

    bool                m_bCustomized;            // This is used for skin editor to disable customized font
    uint32_t            m_nFlagProperties;

    bool                m_bOutlineText;
    CColor              m_clrText, m_clrTextDisbled, m_clrOutlined;
    CColor              m_clrTextOrg, m_clrTextDisbledOrg, m_clrOutlinedOrg;

};
