#include "../GfxLite.h"
#include "GdiGraphicsLite.h"
#include <wingdi.h>


#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY   5
#endif

uint8_t g_byFontQuality = (GetOperationSystemType() >= OPS_WINXP) ? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY;

bool CGdiFont::create(const CFontInfo &font) {
    LOGFONT logFont = {25, 0, 0, 0, FW_REGULAR, false, false, false, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, g_byFontQuality, DEFAULT_PITCH | FF_ROMAN, "Tahoma"};

    logFont.lfHeight = font.getSize();
    logFont.lfWeight = font.getWeight();
    logFont.lfItalic = font.getItalic();
    logFont.lfUnderline = font.isUnderline();
    strcpy_safe(logFont.lfFaceName, CountOf(logFont.lfFaceName), font.getName());

    m_hFont = CreateFontIndirect(&logFont);

    return m_hFont != nullptr;
}


class CGdiTextOut {
public:
    void operator()(CGdiFont *font, cstr_t szClip, int nClip) {
        HFONT hFontOld;

        hFontOld = (HFONT)SelectObject(hdc, font->getHandle());

        ::textOut(hdc, x, y, szClip, nClip);
        ::GetTextExtentPoint32W(hdc, szClip, nClip, &size);
        x += size.cx;

        SelectObject(hdc, hFontOld);
    }

public:
    int                         x, y;
    HDC                         hdc;
    SIZE                        size;

};

class CGdiGetTextExtentPoint32 {
public:
    void operator()(CGdiFont *font, cstr_t szClip, int nClip) {
        HFONT hFontOld;
        SIZE size;

        hFontOld = (HFONT)SelectObject(hdc, font->getHandle());

        ::GetTextExtentPoint32W(hdc, szClip, nClip, &size);
        pSize->cx += size.cx;
        if (size.cy > pSize->cy) {
            pSize->cy = size.cy;
        }

        SelectObject(hdc, hFontOld);
    }

public:
    HDC                         hdc;
    CSize                       *   pSize;

};

//////////////////////////////////////////////////////////////////////
CGdiGraphicsLite::CGdiGraphicsLite() {
    m_hFontOld = nullptr;
    m_bResolveTextEncoding = true;
}

CGdiGraphicsLite::~CGdiGraphicsLite() {
    detach();
}

HDC CGdiGraphicsLite::detach() {
    if (m_hFontOld) {
        SelectObject(m_hdc, m_hFontOld);
        m_hFontOld = nullptr;
    }

    return CGraphics::detach();
}

void CGdiGraphicsLite::setFont(CFontInfo *font) {
    m_fontCollection.setFont(*font);

    if (!m_bResolveTextEncoding) {
        CGdiFont *pGdiFont = m_fontCollection.getFont(this, DEFAULT_CHARSET);
        selectFont(pGdiFont->getHandle());
    }
}

bool CGdiGraphicsLite::textOut(int x, int y, cstr_t szText, int nLen) {
    if (m_bResolveTextEncoding) {
        CGdiTextOut funGdiTextOut;

        funGdiTextOut.x = x;
        funGdiTextOut.y = y;
        funGdiTextOut.hdc = m_hdc;
        m_fontCollection.resovleText(this, szText, nLen, funGdiTextOut);
    } else {
        ::textOut(m_hdc, x, y, szText, nLen);
    }
    return true;
}

bool CGdiGraphicsLite::getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize) {
    if (m_bResolveTextEncoding) {
        CGdiGetTextExtentPoint32 funGdi;

        pSize->cx = 0;
        pSize->cy = 0;
        funGdi.pSize = pSize;
        funGdi.hdc = m_hdc;
        m_fontCollection.resovleText(this, szText, nLen, funGdi);
    } else {
        ::GetTextExtentPoint32W(m_hdc, szText, nLen, pSize);
    }
    return true;
}

void CGdiGraphicsLite::setTextColor(const CColor &color) {
    ::setTextColor(m_hdc, color.get());
}

void CGdiGraphicsLite::setBgColor(const CColor &color) {
    ::SetBkColor(m_hdc, color.get());
}

void CGdiGraphicsLite::enableResolveTextEncoding(bool bEnable) {
    if (!m_bResolveTextEncoding) {
        CGdiFont *pGdiFont = m_fontCollection.getFont(this, DEFAULT_CHARSET);
        selectFont(pGdiFont->getHandle());
    }
}
