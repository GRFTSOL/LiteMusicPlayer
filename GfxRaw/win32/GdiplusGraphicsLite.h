

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
    class GpBrush;
    class RectF;
    class Rect;
    struct GdiplusStartupOutput;
    struct GdiplusStartupInput;
    typedef Rect GpRect;
    typedef RectF GpRectF;
    typedef uint32_t ARGB;
    enum CombineMode;
    typedef float REAL;
    enum TextRenderingHint;
    enum Status;
    enum StringAlignment;
    enum StringTrimming;
};
using namespace Gdiplus;

#define WINGDIPAPI          __stdcall

#define GDIPCONST           const

typedef Status (WINGDIPAPI *FUNGdipCreateFromHDC)(HDC hdc, GpGraphics **graphics);
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromLogfontA) ( HDC hdc, GDIPCONST LOGFONTA *logfont, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromLogfontW) ( HDC hdc, GDIPCONST LOGFONTW *logfont, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateFontFromDC) ( HDC hdc, GpFont **font );
typedef Status (WINGDIPAPI *FUNGdipCreateSolidFill) (ARGB color, GpSolidFill **brush);
typedef Status (WINGDIPAPI *FUNGdipStringFormatGetGenericTypographic) (GpStringFormat **format);
typedef Status (WINGDIPAPI *FUNGdipDeleteStringFormat) (GpStringFormat *format);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatFlags) (GpStringFormat *format, INT flags);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatFlags) (GDIPCONST GpStringFormat *format, INT *flags);
typedef Status (WINGDIPAPI *FUNGdipDeleteFont) (GpFont* font);
typedef Status (WINGDIPAPI *FUNGdipDeleteBrush) (GpBrush *brush);
typedef Status (WINGDIPAPI *FUNGdipDeleteGraphics) (GpGraphics *graphics);
typedef Status (WINGDIPAPI *FUNGdipDrawString) ( GpGraphics *graphics, GDIPCONST WCHAR *string, INT length, GDIPCONST GpFont *font, GDIPCONST RectF *layoutRect, GDIPCONST GpStringFormat *stringFormat, GDIPCONST GpBrush *brush );
typedef Status (WINGDIPAPI *FUNGdipGetClipBoundsI) (GpGraphics *graphics, GpRect *rect);
typedef Status (WINGDIPAPI *FUNGdipSetClipRectI) (GpGraphics *graphics, INT x, INT y, INT width, INT height, CombineMode combineMode);
typedef Status (WINGDIPAPI *FUNGdipSetTextRenderingHint) (GpGraphics *graphics, TextRenderingHint mode);
typedef Status (WINGDIPAPI *FUNGdipMeasureString) ( GpGraphics *graphics, GDIPCONST WCHAR *string, INT length, GDIPCONST GpFont *font, GDIPCONST RectF *layoutRect, GDIPCONST GpStringFormat *stringFormat, RectF *boundingBox, INT *codepointsFitted, INT *linesFilled );
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatAlign) (GpStringFormat *format, StringAlignment align);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatAlign) (GDIPCONST GpStringFormat *format, StringAlignment *align);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatLineAlign) (GpStringFormat *format, StringAlignment align);
typedef Status (WINGDIPAPI *FUNGdipGetStringFormatLineAlign) (GDIPCONST GpStringFormat *format, StringAlignment *align);
typedef Status (WINGDIPAPI *FUNGdipSetStringFormatTrimming) (GpStringFormat *format, StringTrimming trimming);
typedef Status (WINAPI *FUNGdiplusStartup) ( OUT ULONG_PTR *token, const GdiplusStartupInput *input, OUT GdiplusStartupOutput *output);
typedef VOID (WINAPI *FUNGdiplusShutdown) (ULONG_PTR token);


class CGdiplusFont;

//
// GDI Plus方式的画图
//
class CGdiplusGraphicsLite : public CGraphics {
public:
    // typedef bool (*FUNTextOut)(int nXStart, int nYStart, cstr_t lpString, int cbString);
    CGdiplusGraphicsLite();
    ~CGdiplusGraphicsLite();

    virtual void attach(HDC hdc);

    virtual bool textOut(int x, int y, cstr_t szText, int nLen);

    virtual bool getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize);

    virtual void setTextColor(const CColor &color);

    virtual void setBkMode(bool bTransparent);
    virtual void setBgColor(const CColor &color);

    virtual void setFont(CFontInfo *font);

public:
    static bool canSupportGdiplus();
    static bool startup();
    static void shutdown();

protected:
    static HMODULE loadGdiplusDll();

public:
    // GpFont            *font;
    GpGraphics                  *griphics;
    GpSolidFill                 *brush;
    GpStringFormat              *stringFormat;
    bool                        bInitOK;
    HDC                         m_hdcBk;
    HPEN                        m_hPenOld;

    CFontCollection<CGdiplusFont>   m_fontCollection;
    bool                            m_bResolveTextEncoding;

    static FUNGdipCreateFromHDC                     m_sfunGdipCreateFromHDC;
    static FUNGdipCreateFontFromLogfontA            m_sfunGdipCreateFontFromLogfontA;
    static FUNGdipCreateFontFromLogfontW            m_sfunGdipCreateFontFromLogfontW;
    static FUNGdipCreateFontFromDC                  m_sfunGdipCreateFontFromDC;
    static FUNGdipCreateSolidFill                   m_sfunGdipCreateSolidFill;
    static FUNGdipStringFormatGetGenericTypographic m_sfunGdipStringFormatGetGenericTypographic;
    static FUNGdipDeleteStringFormat                m_sfunGdipDeleteStringFormat;
    static FUNGdipSetStringFormatFlags              m_sfunGdipSetStringFormatFlags;
    static FUNGdipGetStringFormatFlags              m_sfunGdipGetStringFormatFlags;
    static FUNGdipDeleteFont                        m_sfunGdipDeleteFont;
    static FUNGdipDeleteBrush                       m_sfunGdipDeleteBrush;
    static FUNGdipDeleteGraphics                    m_sfunGdipDeleteGraphics;
    static FUNGdipDrawString                        m_sfunGdipDrawString;
    static FUNGdipGetClipBoundsI                    m_sfunGdipGetClipBoundsI;
    static FUNGdipSetClipRectI                      m_sfunGdipSetClipRectI;
    static FUNGdipSetTextRenderingHint              m_sfunGdipSetTextRenderingHint;
    static FUNGdipMeasureString                     m_sfunGdipMeasureString;
    static FUNGdipSetStringFormatAlign              m_sfunGdipSetStringFormatAlign;
    static FUNGdipGetStringFormatAlign              m_sfunGdipGetStringFormatAlign;
    static FUNGdipSetStringFormatLineAlign          m_sfunGdipSetStringFormatLineAlign;
    static FUNGdipGetStringFormatLineAlign          m_sfunGdipGetStringFormatLineAlign;
    static FUNGdipSetStringFormatTrimming           m_sfunGdipSetStringFormatTrimming;
    static FUNGdiplusStartup                        m_sfunGdiplusStartup;
    static FUNGdiplusShutdown                       m_sfunGdiplusShutdown;

};

#endif // !defined(GfxRaw_win32_GdiplusGraphicsLite_h)
