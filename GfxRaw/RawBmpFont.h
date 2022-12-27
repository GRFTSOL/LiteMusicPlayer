#pragma once

#if !defined(_RAW_BMP_FONT_H_)
#define _RAW_BMP_FONT_H_

class CRawGraph;
class CRawImage;


#define _CLEAR_TYPE

//
// Glyph pixel format
//
#ifdef _CLEAR_TYPE
enum {
    G_B                         = PIX_B,
    G_G                         = PIX_G,
    G_R                         = PIX_R,
    G_A                         = PIX_A,
    G_PIX_SIZE                  = 4,
};
#else //  _CLEAR_TYPE
enum {
    G_R                         = 0,
    G_G                         = 0,
    G_B                         = 0,
    G_A                         = 0,
    G_PIX_SIZE                  = 1,
};
#endif //  _CLEAR_TYPE

enum {
    MARGIN_FONT                 = 1,
};


struct Glyph {
    Glyph();
    ~Glyph();

    int widthOutlined() { return widthBitmap + marginOutlined; }
    int heightOutlined() { return heightBitmap + marginOutlined; }

    int widthBytes() const { return widthBitmap * G_PIX_SIZE; }
    int widthBytesOutlined() const { return (widthBitmap + marginOutlined) * G_PIX_SIZE; }

    string                      ch;
    uint16_t                    nWidth;             // width of char

    uint8_t                     leftOffset, topOffset; // Left and top offset of bitmap.
    uint16_t                    widthBitmap;
    uint8_t                     heightBitmap;       // width of bitmap
    uint8_t                     marginOutlined;
    bool                        freed;

    uint8_t                     *bitmap;
    uint8_t                     *bitmapOutlined;
    int64_t                     nLastUsedTime;
};


#ifdef _WIN32
#include "win32/RawGlyphBuilder.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/RawGlyphBuilder.h"
#endif

#ifdef _MAC_OS
#include "mac/RawGlyphBuilder.h"
#endif


class CRawGlyphSet : public CFontInfo {
    OBJ_REFERENCE_DECL
public:
    CRawGlyphSet();
    virtual ~CRawGlyphSet();

    virtual bool create(const CFontInfo &font);

    virtual int getHeight() const;

    Glyph *getGlyph(string &ch);

    void clearGlyph();

protected:
    int getGlyphHeight() { return getHeight() + MARGIN_FONT * 2; }

protected:
    friend class CRawGlyphSetMgr;
    typedef map<string, Glyph*>        MAP_GLYPH;

    MAP_GLYPH                   m_mapGlyph;

    int64_t                     m_timeLastClean;

    CRawGlyphBuilder            m_rawGlyphBuilder;

};


class CRawBmpFont {
public:
    enum OverlayMode {
        OM_COLOR,
        OM_PATTERN,
        OM_DUAL_PATTERN,
        OM_PATTERN_COLOR,
    };

    enum ShadowMode {
        SM_NONE,
        SM_OUTLINE,
        SM_SHADOW,
    };

public:
    CRawBmpFont();
    virtual ~CRawBmpFont();

    virtual bool create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline = false);
    virtual bool create(const CFontInfo &font);

    virtual void destroy();

    int getHeight() const;

public:
    bool textOut(CRawGraph *canvas, int x, int y, const CColor &clrText, cstr_t szText, size_t nLen, bool bDrawAlphaChannel);
    bool drawText(CRawGraph *canvas, int x, int y, int width, int xLeftClipOffset, const CColor &clrText, cstr_t szText, size_t nLen, bool bDrawAlphaChannel);
    bool drawTextEx(CRawGraph *canvas, const CRect &rcPos, const CColor &clrText, cstr_t szText, size_t nLen, uint32_t uFormat, bool bDrawAlphaChannel);

    bool outlinedTextOut(CRawGraph *canvas, int x, int y, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, bool bDrawAlphaChannel);
    bool outlinedDrawText(CRawGraph *canvas, int x, int y, int width, int xLeftClipOffset, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, bool bDrawAlphaChannel);
    bool outlinedDrawTextEx(CRawGraph *canvas, const CRect &rcPos, const CColor &clrText, const CColor &clrBorder, cstr_t szText, size_t nLen, uint32_t uFormat, bool bDrawAlphaChannel);

    bool getTextExtentPoint32(cstr_t szText, size_t nLen, CSize *pSize) {
        if (!m_prawGlyphSet) return false;
        pSize->cx = getTextWidth(szText, nLen); pSize->cy = m_prawGlyphSet->getHeight(); return true;
    }

    // void OutlineText
    void setClipBox(CRect &rc);

    void setOverlayPattern(CRawImage *imgPattern);
    void setOverlayPattern(CRawImage *imgPattern1, CRawImage *imgPattern2, int nAlphaPattern1, int nAlphaPattern2);
    void setOverlayPattern(CRawImage *imgPattern1, int nAlphaPattern1, const CColor &clrPattern2, int nAlphaPattern2);
    void useColorOverlay();

    void setShadowMode(ShadowMode shadowMode);

protected:
    int getGlyphHeight() { if (!m_prawGlyphSet) return 0; else return m_prawGlyphSet->getHeight() + MARGIN_FONT * 2; }

    inline void getDrawTextExPosition(cstr_t szText, size_t nLen, const CRect &rcPos, uint32_t uFormat, int &x, int &y, int &width, int &xLeftClipOffset) {
        if (!m_prawGlyphSet) {
            return;
        }

        xLeftClipOffset = 0;

        if (isFlagSet(uFormat, DT_CENTER)) {
            // Align at center
            int nWidthText = getTextWidth(szText, nLen);

            x = (rcPos.right + rcPos.left - nWidthText) / 2;
        } else if (isFlagSet(uFormat, DT_RIGHT)) {
            // Align at right
            int nWidthText = getTextWidth(szText, nLen);

            x = rcPos.right - nWidthText;
        } else {
            x = rcPos.left;
        }

        if (isFlagSet(uFormat, DT_VCENTER)) {
            // Align at vertical center
            y = (rcPos.top + rcPos.bottom - m_prawGlyphSet->getHeight()) / 2;
        } else if (isFlagSet(uFormat, DT_BOTTOM)) {
            // Align at bottom
            y = rcPos.bottom - m_prawGlyphSet->getHeight();
        } else {
            y = rcPos.top;
        }

        if (!isFlagSet(uFormat, DT_NOCLIP)) {
            if (x < rcPos.left) {
                xLeftClipOffset = rcPos.left - x;
            }
            // Y is not CLIPPED yet
            // if (y < rcPos.top)
            //    yTopClipOffset = rcPos.top - y;
            width = rcPos.right - x;
        } else {
            width = getTextWidth(szText, nLen) + m_marginOutlined;
        }
    }

    void splitToMultiLine(const CRect &rcPos, cstr_t szText, size_t nLen, uint32_t uFormat, VecStrings &vLines);

    bool shouldDrawTextEllipsis(cstr_t szText, size_t nLen, int nWidthMax, string &strEllipsis);

    // See DT_PREFIX_TEXT declaration
    bool shouldDrawTextPrefix(cstr_t szText, size_t nLen, string &strPrfix, int &nXPrefix, int &nWidthPrefix);

    int getTextWidth(cstr_t szText, size_t nLen);

protected:
    CRawGlyphSet                *m_prawGlyphSet;

    CRawImage                   *m_imgPattern1, *m_imgPattern2;
    CColor                      m_clrPattern2;
    int                         m_nAlphaPattern1, m_nAlphaPattern2;
    OverlayMode                 m_overlayMode;
    ShadowMode                  m_shadowMode;
    int                         m_marginOutlined;

    CRect                       m_rcClip;

};

#endif // !defined(_RAW_BMP_FONT_H_)
