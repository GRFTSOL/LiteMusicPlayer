#pragma once

#ifndef GfxRaw_win32_GdiGraphicsLite_h
#define GfxRaw_win32_GdiGraphicsLite_h


#include "FontCollection.h"


class CGdiFont {
public:
    CGdiFont() {
        m_hFont = nullptr;
    }
    ~CGdiFont() {
        destroy();
    }

    bool create(CGraphics *canvas, LOGFONTA &logFont) {
        destroy();

        m_hFont = CreateFontIndirectA(&logFont);

        return m_hFont != nullptr;
    }

    bool create(const CFontInfo &font);

    void destroy() {
        if (m_hFont) {
            DeleteObject(m_hFont);
            m_hFont = nullptr;
        }
    }

    HFONT getHandle() { return m_hFont; }

    bool isValid() const { return m_hFont != nullptr; }

protected:
    HFONT                       m_hFont;

};

class CGdiGraphicsLite : public CGraphics {
public:
    CGdiGraphicsLite();
    ~CGdiGraphicsLite();

    virtual HDC detach();

    virtual void setFont(CFontInfo *font);

    virtual bool textOut(int x, int y, cstr_t szText, int nLen);
    virtual bool getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize);

    virtual void setTextColor(const CColor &color);

    virtual void setBgColor(const CColor &color);

    virtual void enableResolveTextEncoding(bool bEnable);

protected:
    void selectFont(HFONT hFont) {
        HFONT hFontOld;

        hFontOld = (HFONT)SelectObject(m_hdc, hFont);
        if (!m_hFontOld) {
            m_hFontOld = hFontOld;
        }
    }

protected:
    CFontCollection<CGdiFont>   m_fontCollection;
    HFONT                       m_hFontOld;
    bool                        m_bResolveTextEncoding;

};

#endif // !defined(GfxRaw_win32_GdiGraphicsLite_h)
