

#include "../GfxLite.h"
#include "GdiplusGraphicsLite.h"


//typedef uint32_t *    ULONG_PTR;
#define iterator            Gdiplusiterator
#define list                Gdipluslist
#define map                 Gdiplusmap
#include "../../third-parties/GDIPlus/Include/GdiPlus.h"
#undef Gdiplusiterator
#undef Gdipluslist
#undef Gdiplusmap
#include <atlbase.h>


using namespace Gdiplus;
using namespace DllExports;

ARGB makeARGB(IN uint8_t a,
    IN uint8_t r,
    IN uint8_t g,
    IN uint8_t b) {
    return (((ARGB) (b) <<  BLUE_SHIFT) |
        ((ARGB) (g) << GREEN_SHIFT) |
        ((ARGB) (r) <<   RED_SHIFT) |
        ((ARGB) (a) << ALPHA_SHIFT));
}


class CGdiplusFont {
public:
    CGdiplusFont() {
        m_font = nullptr;
    }
    ~CGdiplusFont() {
        destroy();
    }

    bool create(CGraphics *canvas, LOGFONTA &logFont) {
        Status ret;

        destroy();

        ret = CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA(canvas->getHandle(), &logFont, &m_font);
        if (ret == NotTrueTypeFont) {
            // 选择默认的TRUEType字体！
            LOGFONTA logfontNew = logFont;
            strcpy_safe(logfontNew.lfFaceName, CountOf(logfontNew.lfFaceName), "Tahoma");
            ret = CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA(canvas->getHandle(), &logfontNew, &m_font);
        }

        return ret == Ok;
    }

    void destroy() {
        if (m_font) {
            CGdiplusGraphicsLite::m_sfunGdipDeleteFont(m_font);
            m_font = nullptr;
        }
    }

    GpFont *getHandle() { return m_font; }

protected:
    GpFont                      *m_font;

};

class CGdiplusTextOut {
public:
    void operator()(CGdiplusFont *font, cstr_t szClip, int nClip) {
        Status ret;
        RectF rcLayout(rect.X, rect.Y, 0.0f, 0.0f);
        RectF rcBoundBox;

        ret = CGdiplusGraphicsLite::m_sfunGdipDrawString(canvas->griphics, szClip, nClip, font->getHandle(), &rect, canvas->stringFormat, canvas->brush);
        if (ret != Ok) {
            DBG_LOG1("m_sfunGdipSetTextRenderingHint FAILED: %d!", ret);
        }
        CGdiplusGraphicsLite::m_sfunGdipMeasureString(canvas->griphics, szClip, nClip, font->getHandle(), &rcLayout, canvas->stringFormat, &rcBoundBox, nullptr, nullptr);
        rect.X += rcBoundBox.width;
    }

public:
    RectF                       rect;
    CGdiplusGraphicsLite        *canvas;

};

class CGdiplusMeasureString {
public:
    void operator()(CGdiplusFont *font, cstr_t szClip, int nClip) {
        RectF rcLayout(0.0f, 0.0f, 0.0f, 0.0f);
        RectF rcBoundBox;

        CGdiplusGraphicsLite::m_sfunGdipMeasureString(canvas->griphics, szClip, nClip, font->getHandle(), &rcLayout, canvas->stringFormat, &rcBoundBox, nullptr, nullptr);
        cx += rcBoundBox.width;
        if (rcBoundBox.height > cy) {
            cy = rcBoundBox.height;
        }
    }

public:
    REAL                        cx, cy;
    CGdiplusGraphicsLite        *canvas;

};

typedef Status (WINGDIPAPI *FUNGdipGetStringFormatTrimming) (GpStringFormat *format, StringTrimming *trimming);

FUNGdipCreateFromHDC CGdiplusGraphicsLite::m_sfunGdipCreateFromHDC = nullptr;
FUNGdipCreateFontFromLogfontA CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontA = nullptr;
// FUNGdipCreateFontFromLogfontW            CGdiplusGraphicsLite::m_sfunGdipCreateFontFromLogfontW = nullptr;
FUNGdipCreateFontFromDC CGdiplusGraphicsLite::m_sfunGdipCreateFontFromDC = nullptr;
FUNGdipCreateSolidFill CGdiplusGraphicsLite::m_sfunGdipCreateSolidFill = nullptr;
FUNGdipStringFormatGetGenericTypographic CGdiplusGraphicsLite::m_sfunGdipStringFormatGetGenericTypographic = nullptr;
FUNGdipDeleteStringFormat CGdiplusGraphicsLite::m_sfunGdipDeleteStringFormat = nullptr;
FUNGdipSetStringFormatFlags CGdiplusGraphicsLite::m_sfunGdipSetStringFormatFlags = nullptr;
FUNGdipGetStringFormatFlags CGdiplusGraphicsLite::m_sfunGdipGetStringFormatFlags = nullptr;
FUNGdipDeleteFont CGdiplusGraphicsLite::m_sfunGdipDeleteFont = nullptr;
FUNGdipDeleteBrush CGdiplusGraphicsLite::m_sfunGdipDeleteBrush = nullptr;
FUNGdipDeleteGraphics CGdiplusGraphicsLite::m_sfunGdipDeleteGraphics = nullptr;
FUNGdipDrawString CGdiplusGraphicsLite::m_sfunGdipDrawString = nullptr;
FUNGdipGetClipBoundsI CGdiplusGraphicsLite::m_sfunGdipGetClipBoundsI = nullptr;
FUNGdipSetClipRectI CGdiplusGraphicsLite::m_sfunGdipSetClipRectI = nullptr;
FUNGdipSetTextRenderingHint CGdiplusGraphicsLite::m_sfunGdipSetTextRenderingHint = nullptr;
FUNGdipMeasureString CGdiplusGraphicsLite::m_sfunGdipMeasureString = nullptr;
FUNGdipSetStringFormatAlign CGdiplusGraphicsLite::m_sfunGdipSetStringFormatAlign = nullptr;
FUNGdipGetStringFormatAlign CGdiplusGraphicsLite::m_sfunGdipGetStringFormatAlign = nullptr;
FUNGdipSetStringFormatLineAlign CGdiplusGraphicsLite::m_sfunGdipSetStringFormatLineAlign = nullptr;
FUNGdipGetStringFormatLineAlign CGdiplusGraphicsLite::m_sfunGdipGetStringFormatLineAlign = nullptr;
FUNGdipSetStringFormatTrimming CGdiplusGraphicsLite::m_sfunGdipSetStringFormatTrimming = nullptr;
FUNGdipGetStringFormatTrimming m_sfunGdipGetStringFormatTrimming = nullptr;
FUNGdiplusStartup CGdiplusGraphicsLite::m_sfunGdiplusStartup = nullptr;
FUNGdiplusShutdown CGdiplusGraphicsLite::m_sfunGdiplusShutdown = nullptr;

// #ifdef _UNICODE
// #define m_sfunGdipCreateFontFromLogfont        m_sfunGdipCreateFontFromLogfontW
// #else
#define m_sfunGdipCreateFontFromLogfont m_sfunGdipCreateFontFromLogfontA
// #endif

#ifdef _WIN32
ULONG_PTR g_gdiplusToken = nullptr;
#endif

CGdiplusGraphicsLite::CGdiplusGraphicsLite() {
    m_hdcBk = nullptr;
    griphics = nullptr;
    brush = nullptr;
    stringFormat = nullptr;
    bInitOK = false;
    m_bResolveTextEncoding = true;
}

CGdiplusGraphicsLite::~CGdiplusGraphicsLite() {
    if (stringFormat) {
        m_sfunGdipDeleteStringFormat(stringFormat);
    }
    if (brush) {
        m_sfunGdipDeleteBrush(brush);
    }
    if (griphics) {
        m_sfunGdipDeleteGraphics(griphics);
    }

    if (m_hdcBk) {
        DeleteDC(m_hdcBk);
    }
}

void CGdiplusGraphicsLite::attach(HDC hdc) {
    if (!m_sfunGdipCreateFromHDC) {
        //
        // 载入地址
        HMODULE hModule;

        hModule = loadGdiplusDll();
        if (!hModule) {
            DBG_LOG0("load Gdiplus Dll FAILED!");
            return;
        }

        m_sfunGdipCreateFromHDC = (FUNGdipCreateFromHDC )GetProcAddress(hModule, "GdipCreateFromHDC");
        m_sfunGdipCreateFontFromLogfontA = (FUNGdipCreateFontFromLogfontA )GetProcAddress(hModule, "GdipCreateFontFromLogfontA");
        // m_sfunGdipCreateFontFromLogfontW        = (FUNGdipCreateFontFromLogfontW            )GetProcAddress(hModule, "GdipCreateFontFromLogfontW");
        m_sfunGdipCreateFontFromDC = (FUNGdipCreateFontFromDC )GetProcAddress(hModule, "GdipCreateFontFromDC");
        m_sfunGdipCreateSolidFill = (FUNGdipCreateSolidFill )GetProcAddress(hModule, "GdipCreateSolidFill");
        //        m_sfunGdipStringFormatGetGenericDefault = (FUNGdipStringFormatGetGenericDefault    )GetProcAddress(hModule, "GdipStringFormatGetGenericDefault");
        m_sfunGdipStringFormatGetGenericTypographic = (FUNGdipStringFormatGetGenericTypographic )GetProcAddress(hModule, "GdipStringFormatGetGenericTypographic");
        m_sfunGdipDeleteStringFormat = (FUNGdipDeleteStringFormat )GetProcAddress(hModule, "GdipDeleteStringFormat");
        m_sfunGdipSetStringFormatFlags = (FUNGdipSetStringFormatFlags )GetProcAddress(hModule, "GdipSetStringFormatFlags");
        m_sfunGdipGetStringFormatFlags = (FUNGdipGetStringFormatFlags )GetProcAddress(hModule, "GdipGetStringFormatFlags");
        m_sfunGdipDeleteFont = (FUNGdipDeleteFont )GetProcAddress(hModule, "GdipDeleteFont");
        m_sfunGdipDeleteBrush = (FUNGdipDeleteBrush )GetProcAddress(hModule, "GdipDeleteBrush");
        m_sfunGdipDeleteGraphics = (FUNGdipDeleteGraphics )GetProcAddress(hModule, "GdipDeleteGraphics");
        m_sfunGdipDrawString = (FUNGdipDrawString )GetProcAddress(hModule, "GdipDrawString");
        m_sfunGdipGetClipBoundsI = (FUNGdipGetClipBoundsI )GetProcAddress(hModule, "GdipGetClipBoundsI");
        m_sfunGdipSetClipRectI = (FUNGdipSetClipRectI )GetProcAddress(hModule, "GdipSetClipRectI");
        m_sfunGdipSetTextRenderingHint = (FUNGdipSetTextRenderingHint )GetProcAddress(hModule, "GdipSetTextRenderingHint");
        m_sfunGdipMeasureString = (FUNGdipMeasureString )GetProcAddress(hModule, "GdipMeasureString");
        m_sfunGdipSetStringFormatAlign = (FUNGdipSetStringFormatAlign )GetProcAddress(hModule, "GdipSetStringFormatAlign");
        m_sfunGdipGetStringFormatAlign = (FUNGdipGetStringFormatAlign )GetProcAddress(hModule, "GdipGetStringFormatAlign");
        m_sfunGdipSetStringFormatLineAlign = (FUNGdipSetStringFormatLineAlign )GetProcAddress(hModule, "GdipSetStringFormatLineAlign");
        m_sfunGdipGetStringFormatLineAlign = (FUNGdipGetStringFormatLineAlign )GetProcAddress(hModule, "GdipGetStringFormatLineAlign");
        m_sfunGdipSetStringFormatTrimming = (FUNGdipSetStringFormatTrimming )GetProcAddress(hModule, "GdipSetStringFormatTrimming");
        m_sfunGdipGetStringFormatTrimming = (FUNGdipGetStringFormatTrimming )GetProcAddress(hModule, "GdipGetStringFormatTrimming");
        if (!m_sfunGdipCreateFromHDC || !m_sfunGdipCreateFontFromLogfontA
            // || !m_sfunGdipCreateFontFromLogfontW
            || !m_sfunGdipCreateFontFromDC || !m_sfunGdipCreateSolidFill
            || !m_sfunGdipStringFormatGetGenericTypographic || !m_sfunGdipDeleteStringFormat
            || !m_sfunGdipGetStringFormatFlags || !m_sfunGdipSetStringFormatFlags
            || !m_sfunGdipDeleteFont || !m_sfunGdipDeleteBrush
            || !m_sfunGdipDeleteGraphics || !m_sfunGdipSetTextRenderingHint
            || !m_sfunGdipSetTextRenderingHint || !m_sfunGdipDrawString
            || !m_sfunGdipGetClipBoundsI || !m_sfunGdipSetClipRectI
            || !m_sfunGdipSetTextRenderingHint || !m_sfunGdipMeasureString
            || !m_sfunGdipSetStringFormatAlign || !m_sfunGdipGetStringFormatAlign
            || !m_sfunGdipSetStringFormatLineAlign || !m_sfunGdipGetStringFormatLineAlign
            || !m_sfunGdipSetStringFormatTrimming || !m_sfunGdipGetStringFormatTrimming) {
            return;
        }
    }

    m_hdc = hdc;

    Status ret;

    ret = m_sfunGdipCreateFromHDC(hdc, &griphics);
    if (ret != Ok) {
        DBG_LOG1("m_sfunGdipCreateFromHDC FAILED: %d!", ret);
        return;
    }
    ret = m_sfunGdipStringFormatGetGenericTypographic(&stringFormat);
    if (ret != Ok) {
        DBG_LOG1("m_sfunGdipStringFormatGetGenericTypographic FAILED: %d!", ret);
        return;
    }
    INT flag;
    ret = m_sfunGdipGetStringFormatFlags(stringFormat, &flag);
    flag &= ~StringFormatFlagsNoClip;
    ret = m_sfunGdipSetStringFormatFlags(stringFormat, flag | StringFormatFlagsMeasureTrailingSpaces | StringFormatFlagsNoWrap);
    //     StringTrimming    strTrimming;
    //     ret = m_sfunGdipGetStringFormatTrimming(stringFormat, &strTrimming);
    ret = m_sfunGdipSetStringFormatTrimming(stringFormat, StringTrimmingNone);
    //    ret = GdipCreateFontFromDC(hdc, &font);
    //    if (ret != Ok)
    //        return;
    //    clrText = GetTextColor(hdc);
    //    ret = GdipCreateSolidFill(makeARGB(255, getRValue(clrText), getGValue(clrText), getBValue(clrText)), &brush);
    //    if (ret != Ok)
    //        return;
    bInitOK = true;
}

bool CGdiplusGraphicsLite::textOut(int x, int y, cstr_t szText, int nLen) {
    Status ret;
    RectF rect((float)x, (float)y, (float)2000, (float)300);

    if (!bInitOK) {
        return false;
    }

    ret = m_sfunGdipSetTextRenderingHint(griphics, TextRenderingHintAntiAlias);
    if (ret != Ok) {
        DBG_LOG1("m_sfunGdipSetTextRenderingHint FAILED: %d!", ret);
        return false;
    }

    if (m_bResolveTextEncoding) {
        CGdiplusTextOut funGdiplus;

        funGdiplus.canvas = this;
        funGdiplus.rect = rect;
        m_fontCollection.resovleText(this, szText, nLen, funGdiplus);
    } else {
        CGdiplusFont *font;
        font = m_fontCollection.getFont(this, DEFAULT_CHARSET);
        ret = m_sfunGdipDrawString(griphics, szText, nLen, font->getHandle(), &rect, stringFormat, brush);
    }
    //     if (ret != Ok)
    //     {
    //         DBG_LOG1("m_sfunGdipDrawString FAILED: %d!", ret);
    //         return false;
    //     }

    return true;
}

bool CGdiplusGraphicsLite::getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize) {
    //    RectF        rcLayout(0, 0, 2000, 300);
    //    RectF        rcBoundBox;
    if (!bInitOK) {
        pSize->cx = 0;
        pSize->cy = 0;
        return false;
    }

    if (m_bResolveTextEncoding) {
        CGdiplusMeasureString funGdiplus;

        funGdiplus.canvas = this;
        funGdiplus.cx = 0;
        funGdiplus.cy = 0;

        m_fontCollection.resovleText(this, szText, nLen, funGdiplus);

        pSize->cx = (INT)funGdiplus.cx;
        pSize->cy = (INT)funGdiplus.cy;
    } else {
        CGdiplusFont *font;
        RectF rcLayout(0.0f, 0.0f, 0.0f, 0.0f);
        RectF rcBoundBox;

        font = m_fontCollection.getFont(this, DEFAULT_CHARSET);
        if (m_sfunGdipMeasureString(griphics, szText, nLen, font->getHandle(), &rcLayout, stringFormat, &rcBoundBox, nullptr, nullptr) != Ok) {
            return false;
        }

        pSize->cx = (INT)rcBoundBox.width;
        pSize->cy = (INT)rcBoundBox.height;
    }

    return true;
}

void CGdiplusGraphicsLite::setTextColor(const CColor &color) {
    Status ret;

    if (brush) {
        m_sfunGdipDeleteBrush(brush);
        brush = nullptr;
    }

    ret = m_sfunGdipCreateSolidFill(makeARGB(255, getRValue(color.get()), getGValue(color.get()), getBValue(color.get())), &brush);
    if (ret != Ok) {
        DBG_LOG1("m_sfunGdipCreateSolidFill FAILED: %d!", ret);
        bInitOK = false;
    }
}

void CGdiplusGraphicsLite::setBkMode(bool bTransparent) {
    ::setBkMode(m_hdc, bTransparent ? TRANSPARENT : OPAQUE);
}

void CGdiplusGraphicsLite::setBgColor(const CColor &color) {
    ::SetBkColor(m_hdc, color.get());
}

void CGdiplusGraphicsLite::setFont(CFontInfo *font) {
    m_fontCollection.setFont(*font);
}

bool CGdiplusGraphicsLite::canSupportGdiplus() {
    int nRet;
    nRet = GetOperationSystemType();
    if (nRet == OPS_WIN95 || nRet == OPS_WINNT4) {
        return false;
    }

    HMODULE hModule;

    hModule = loadGdiplusDll();
    if (!hModule) {
        return false;
    }

    FreeLibrary(hModule);
    return true;
}

HMODULE CGdiplusGraphicsLite::loadGdiplusDll() {
    HMODULE hModule;

    hModule = LoadLibrary("Gdiplus.dll");
    if (!hModule) {
        char szDll[MAX_PATH];

        GetAppResourceDir(szDll);
        strcat_safe(szDll, CountOf(szDll), "Gdiplus.dll");
        hModule = LoadLibrary(szDll);
    }
    return hModule;
}

bool CGdiplusGraphicsLite::startup() {
    GdiplusStartupInput gdiplusStartupInput;

    if (!m_sfunGdiplusStartup || !m_sfunGdiplusShutdown) {
        HMODULE hModule;
        hModule = loadGdiplusDll();
        if (!hModule) {
            return false;
        }
        m_sfunGdiplusStartup = (FUNGdiplusStartup)GetProcAddress(hModule, "GdiplusStartup");
        m_sfunGdiplusShutdown = (FUNGdiplusShutdown)GetProcAddress(hModule, "GdiplusShutdown");
    }

    // Initialize GDI+.
    if (m_sfunGdiplusStartup && g_gdiplusToken == nullptr) {
        return m_sfunGdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr) == Ok;
    }
    return false;
}

void CGdiplusGraphicsLite::shutdown() {
    if (m_sfunGdiplusShutdown) {
        m_sfunGdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = nullptr;
    }
}
