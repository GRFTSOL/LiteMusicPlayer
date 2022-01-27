#include <pango/pangocairo.h>
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"

namespace GfxRaw
{

inline uint8_t RGBToGray(int r, int g, int b)
{
    // 5 = 四舍五入
    return (r * 3 + g * 6 + b + 5) / 10;
}

CRawGlyphBuilder::CRawGlyphBuilder()
{
    m_cr = nullptr;
    m_surface = nullptr;
    m_nWidthGraph = 0;
}

CRawGlyphBuilder::~CRawGlyphBuilder()
{
    cairo_destroy(m_cr);
    cairo_surface_destroy(m_surface);
}

void CRawGlyphBuilder::init(const CFontInfo &font)
{
    if (m_cr && m_font.isSame(font))
        return;

    m_font.create(font);

    cairo_status_t status;

    m_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
        getHeightBitmap() * 2, getHeightBitmap() * 2);
    m_cr = cairo_create(m_surface);

    m_nWidthGraph = getHeightBitmap() * 2;
}

Glyph *CRawGlyphBuilder::buildGlyph(string &ch)
{
    PangoLayout *layout;
    PangoFontDescription *desc;
    int width, height;

    // Clear graph
    cairo_set_source_rgb(m_cr, 0, 0, 0);
    cairo_paint(m_cr);

    // cairo_translate(m_cr, m_font.getHeight(), m_font.getHeight());

    // create font
    // desc = pango_font_description_new();
    pango_font_description_set_family(desc, m_font.getName());
    pango_font_description_set_size(desc, m_font.getSize());
    pango_font_description_set_weight(desc, (PangoWeight)m_font.getWeight());
    pango_font_description_set_style(desc, (PangoStyle)m_font.getItalic());
//    #define FONT "Sans Bold 12"
//    desc = pango_font_description_from_string (FONT);

    // create a PangoLayout, set the font and text
    layout = pango_cairo_create_layout (m_cr);

    pango_layout_set_text (layout, ch, -1);
    pango_layout_set_font_description (layout, desc);
    pango_font_description_free (desc);

    // Draw text
    cairo_set_source_rgb (m_cr, 1.0, 1.0, 1.0);
    // pango_cairo_update_layout (m_cr, layout);
    pango_layout_get_size (layout, &width, &height);
    pango_cairo_show_layout (m_cr, layout);

    g_object_unref (layout);

//     cairo_status_t status;
//     CStrPrintf    strFile("Char%d.png", ch[0]);
// 
//     status = cairo_surface_write_to_png (m_surface, strFile.c_str());
//     if (status != CAIRO_STATUS_SUCCESS)
//     {
//         g_printerr ("Could not save png to '%s'\n", strFile.c_str());
//     }


    width /= PANGO_SCALE;
    height /= PANGO_SCALE;

    int                nWidth;
    int                x, y;
    uint8_t            *pDst, *pRowDst, *pSrc, *pRowSrc;

    DBG_LOG3("glyph: %c, w: %d, h: %d", (cstr_t)ch, width, height);
    DBG_LOG1("Font size: %d", m_font.getSize());
    DBG_LOG3("Format: %d, %d, %d", cairo_image_surface_get_format (m_surface), CAIRO_FORMAT_ARGB32, CAIRO_FORMAT_RGB24);
    // assert(height == m_font.getHeight());

    agg::rendering_buffer    bufGraph(cairo_image_surface_get_data(m_surface), cairo_image_surface_get_width(m_surface), cairo_image_surface_get_height(m_surface), cairo_image_surface_get_stride(m_surface));

    nWidth = width + MARGIN_FONT * 2;

    Glyph        *glyph = new Glyph;
    glyph->nWidth = (int16_t)width;
    glyph->nWidthBitmap = nWidth;
    glyph->bitmap = new uint8_t[glyph->widthBytes() * getHeightBitmap()];

    pRowSrc = bufGraph.row_ptr(0);
    pRowDst = glyph->bitmap;
    for (y = 0; y < getHeightBitmap(); y++)
    {
        pDst = pRowDst;
        pSrc = pRowSrc;
        for (x = 0; x < glyph->nWidthBitmap; x++)
        {
            //             pDst[G_R] = 255;
            //             pDst[G_G] = 255;
            //             pDst[G_B] = 255;
            //             pDst[G_A] = 255;
            pDst[G_R] = pSrc[0];
            pDst[G_G] = pSrc[1];
            pDst[G_B] = pSrc[2];
            pDst[G_A] = RGBToGray(pSrc[G_R], pSrc[G_G], pSrc[G_B]);
            pDst += G_PIX_SIZE;
            pSrc += 4;
        }
        pRowSrc += bufGraph.stride();
        pRowDst += glyph->widthBytes();
    }

    return glyph;
}

};
