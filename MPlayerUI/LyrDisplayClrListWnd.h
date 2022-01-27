#pragma once


#include "LyricShowObj.h"

class CLyrDisplayClrListWnd : public IPopupSkinWndNotify
{
public:
    struct FontClrOpt
    {
        FontClrOpt(COLORREF clr) {
            this->obm = OBM_COLOR;
            this->clr[0] = clr;
        }
        FontClrOpt(COLORREF clr0, COLORREF clr1, COLORREF clr2) {
            this->obm = OBM_GRADIENT_COLOR;
            this->clr[0] = clr0;
            this->clr[1] = clr1;
            this->clr[2] = clr2;
        }
        FontClrOpt(cstr_t szFile) {
            this->obm = OBM_PATTERN;
            strPatternFile = szFile;
        }

        OverlayBlendingMode        obm;                // Overlay blending mode
        COLORREF                clr[3];
        string                    strPatternFile;
    };
    typedef vector<FontClrOpt>    VecFontClr;

public:
    CLyrDisplayClrListWnd();

    void create(CSkinWnd *pWnd, CRect &rc, bool bHighLight, bool bFloatingLyr, IPopupSkinWndNotify *pNotify);

    virtual void popupSkinWndOnSelected();

protected:
    enum {
        HEIGHT_COLOR = 40,
        WIDTH_COLOR = 100
    };

    void selectCurrentSetting();

    int findGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2);

    int findPattern(cstr_t szPatternFile);

    void addPureColor(COLORREF clr);

    void addGradientColors();

    void addGradientColor(COLORREF clr0, COLORREF clr1, COLORREF clr2);

    void addPatternImages();

    void addImage(RawImageData *image);

protected:
    bool                m_bInitialized;
    CSkinWnd            *m_pWnd;
    IPopupSkinWndNotify    *m_pNotify;
    CPopupSkinListWnd    m_popupListWnd;
    int                    m_nWidthMax;

    VecFontClr            m_vFontClr;

    EventType            m_et;
    string                m_strSectName;
    bool                m_bHilight;

};
