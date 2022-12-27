#ifndef _RAW_GRAPH_DATA_H_
#define _RAW_GRAPH_DATA_H_

#pragma once

class CRawGraphData {
public:
    CRawGraphData();
    virtual ~CRawGraphData();

    virtual bool create(int cx, int cy, Window *window, int nBitCount = 32);

    virtual void destroy();

    int width() const { return m_imageData.width; }
    int height() const { return m_imageData.height; }

    RawImageData *getRawBuff() { return &m_imageData; }
    uint8_t *getBuff() { return m_imageData.buff; }

    bool isValid() { return m_imageData.buff != nullptr; }

    virtual void drawToWindow(int xdest, int ydest, int width, int height, int xsrc, int ysrc);
    virtual void drawToWindowStretch(int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc);

    //     virtual void DrawGraphicsAlphaBlend(CGraphics *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, uint8_t nAlpha, bool bPerPixelAlpha);
    //     virtual void DrawGraphicsStretchAlphaBlend(CGraphics *pSrc, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc, uint8_t nAlpha, bool bPerPixelAlpha);

public:
    // For win32 only
    HDC getHandle() const { return m_hdc; }

protected:
    Window                      *m_window;
    RawImageData                m_imageData;
    HDC                         m_hdc;
    HBITMAP                     m_bmpMem;
    HBITMAP                     m_bmpOld;

    BITMAPINFO                  m_bmpInfo;

};

#endif // _RAW_GRAPH_DATA_H_
