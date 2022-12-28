#include "MPlayerApp.h"
#include "DlgChooseFont.h"


// #define FLAG_BOLD        1
// #define FLAG_ITALIC        (0x1 << 1)
//


CDlgChooseFont::CDlgChooseFont() {
}

CDlgChooseFont::~CDlgChooseFont() {

}

int CDlgChooseFont::doModal(Window *pWndParent, cstr_t szFontFaceName, int nFontSize, int nWeight, int nItalic) {
    LOGFONT lgfont = { 0 };
    CHOOSEFONT choosefont = { 0 };

    m_strFontFaceName = szFontFaceName;
    m_nFontSize = nFontSize;
    m_weight = nWeight;
    m_nItalic = nItalic;

    lgfont.lfHeight = nFontSize;
    lgfont.lfWeight = nWeight;
    lgfont.lfItalic = nItalic;
    strcpy_safe(lgfont.lfFaceName, CountOf(lgfont.lfFaceName), szFontFaceName);

    choosefont.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCALABLEONLY | CF_SCREENFONTS | CF_NOVERTFONTS;// | CF_EFFECTS;
    choosefont.lpLogFont = &lgfont;
    choosefont.hwndOwner = pWndParent->getHandle();
    choosefont.lStructSize = sizeof(choosefont);
    if (ChooseFont(&choosefont)) {
        m_strFontFaceName = lgfont.lfFaceName;
        m_nFontSize = lgfont.lfHeight;
        m_weight = lgfont.lfWeight;
        m_nItalic = lgfont.lfItalic;

        return IDOK;
    }

    return IDCANCEL;
}

cstr_t CDlgChooseFont::getFaceName() {
    return m_strFontFaceName.c_str();
}

int CDlgChooseFont::getSize() {
    return m_nFontSize;
}

int CDlgChooseFont::getWeight() {
    return m_weight;
}

int CDlgChooseFont::getItalic() {
    return m_nItalic;
}
