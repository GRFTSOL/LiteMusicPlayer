#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include "../../third-parties/Agg/include/agg_scanline_p.h"
#include "../../third-parties/Agg/include/agg_renderer_scanline.h"
#include "../../third-parties/Agg/include/agg_pixfmt_rgba.h"
#include "../../third-parties/Agg/include/agg_pixfmt_rgb.h"
#include "../../third-parties/Agg/include/agg_rasterizer_scanline_aa.h"
#include "GfxRaw.h"
#include "RawBmpFont.h"
#include "RawGlyphBuilder.h"


#define MEM_GRAPH_BITS    32

extern bool    g_bAntialiasFontEnabled;

CRawGlyphBuilder::CRawGlyphBuilder()
{
    m_attributes = nullptr;
}

CRawGlyphBuilder::~CRawGlyphBuilder()
{
    if (m_attributes)
        CFRelease(m_attributes);
}

void CRawGlyphBuilder::init(const CFontInfo &font)
{
    if (m_fontMemGraph.isValid())
    {
        if (m_font.isSame(font))
            return;
    }

    m_fontMemGraph.destroy();

    m_font.create(font);

    {
        m_nLineHeight = m_font.getHeight();
        
        m_fontMemGraph.create(getHeightBitmap() * 2, getHeightBitmap(), nullptr, MEM_GRAPH_BITS);

        CGContextSetTextMatrix(m_fontMemGraph.getHandle(), CGAffineTransformMakeScale(1.0, -1.0));

        CGColorSpaceRef rgbColorSpace = CGColorSpaceCreateDeviceRGB();
        CGFloat components[] = { 1.0, 1.0, 1.0, 1.0 };
        CGColorRef red = CGColorCreate(rgbColorSpace, components);
        CGColorSpaceRelease(rgbColorSpace);
        
        // Initialize string, font, and context
        CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName };
        CFTypeRef values[] = { m_font.getHandle(), red };
        
        m_attributes = CFDictionaryCreate(kCFAllocatorDefault, (const void**)&keys,
                           (const void**)&values, sizeof(keys) / sizeof(keys[0]),
                           &kCFTypeDictionaryKeyCallBacks,
                           &kCFTypeDictionaryValueCallBacks);
        
        
    }
}

int CRawGlyphBuilder::getHeight() const
{
    return m_font.getHeight();
}

void CRawGlyphBuilder::destroy()
{
    m_fontMemGraph.destroy();
    m_font.destroy();
}

void copyBitmpDataFromGraphBuffer(Glyph *pGlyph, agg::pixfmt_rgba32 &bufGraph);

int CRawGlyphBuilder::getHeightBitmap()
{
    return m_nLineHeight + MARGIN_FONT * 2;
}

Glyph *CRawGlyphBuilder::buildGlyph(string &ch)
{
    CSize size;
    RawImageData *rawData = m_fontMemGraph.getRawBuff();

    agg::rendering_buffer    bufGraph(rawData->buff, rawData->width, rawData->height, rawData->stride);

    int nHeightBitmap = getHeightBitmap();

    uint8_t *pRowSrc = bufGraph.row_ptr(0);
    for (int y = 0; y < nHeightBitmap; y++)
    {
        memset(pRowSrc, 0, bufGraph.stride_abs());
        pRowSrc += bufGraph.stride();
    }
    
    {
        CFStringRef    string = CFStringCreateWithBytes(nullptr, (UInt8*)(const char*)ch.c_str(), ch.size(), kCFStringEncodingUTF8, false);
        if (string)
        {
            CFAttributedStringRef attrString = CFAttributedStringCreate(kCFAllocatorDefault, string, m_attributes);
            CFRelease(string);
            
            CTLineRef line = CTLineCreateWithAttributedString(attrString);
            
            // Set text position and draw the line into the graphics context
            CGContextSetTextPosition(m_fontMemGraph.getHandle(), MARGIN_FONT, MARGIN_FONT + m_nLineHeight - m_nLineHeight / 6);
            CGPoint ptStart = CGContextGetTextPosition(m_fontMemGraph.getHandle());
            
            CTLineDraw(line, m_fontMemGraph.getHandle());

            CGPoint ptEnd = CGContextGetTextPosition(m_fontMemGraph.getHandle());
            
            size.cx = ptEnd.x - ptStart.x;
            
            CFRelease(line);
            CFRelease(attrString);
        }
        else
        {
            size.cx = m_font.getHeight() / 2;
        }
    }

    Glyph        *glyph = new Glyph;
    glyph->ch = ch;
    glyph->nWidth = (uint8_t)size.cx;

    agg::pixfmt_rgba32        pixf(bufGraph);
    copyBitmpDataFromGraphBuffer(glyph, pixf);

    return glyph;
}
