

#ifndef GfxRaw_win32_GdiplusGraphicsLite_h
#define GfxRaw_win32_GdiplusGraphicsLite_h

#pragma once

#include "FontCollection.h"


#ifndef __int3264
typedef uint32_t * ULONG_PTR;
#endif

namespace Gdiplus
{
    class GpFont;
    class GpGraphics;
    class GpSolidFill;
    class GpStringFormat;
};
using namespace Gdiplus;


class CGdiplusFont;
using CGdiplusFontPtr = std::shared_ptr<CGdiplusFont>;

//
// GDI Plus方式的画图
//
class CGdiplusGraphicsLite {
public:
    // typedef bool (*FUNTextOut)(int nXStart, int nYStart, cstr_t lpString, int cbString);
    CGdiplusGraphicsLite();
    ~CGdiplusGraphicsLite();

    void attach(HDC hdc);
    HDC getHandle() const { return m_hdc; }

    bool textOut(int x, int y, cwstr_t szText, int nLen);

    bool getTextExtentPoint32(cwstr_t szText, int nLen, CSize *pSize);

    void setTextColor(const CColor &color);

    void setBkMode(bool bTransparent);
    void setBgColor(const CColor &color);

    void setFont(const CGdiplusFontPtr &font) { m_font = font; }

    CGdiplusFontPtr createFont(FontInfoEx &font, bool isLatin);

public:
    static bool canSupportGdiplus();
    static bool startup();
    static void shutdown();

protected:
    static HMODULE loadGdiplusDll();

public:
    GpGraphics                      *griphics = NULL;
    GpSolidFill                     *brush = NULL;
    GpStringFormat                  *stringFormat = NULL;
    bool                            bInitOK = NULL;
    HDC                             m_hdc = NULL;
    HDC                             m_hdcBk = NULL;
    HPEN                            m_hPenOld = NULL;
    CGdiplusFontPtr                 m_font;

};

#endif // !defined(GfxRaw_win32_GdiplusGraphicsLite_h)
