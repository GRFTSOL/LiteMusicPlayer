#ifndef _RAW_GRAPH_DATA_H_
#define _RAW_GRAPH_DATA_H_

#pragma once

class CRawGraphData {
public:
    CRawGraphData();
    virtual ~CRawGraphData();

    virtual bool create(int cx, int cy, WindowHandle handle, int nBitCount = 32);

    virtual void destroy();

    int width() const { return m_imageData.width; }
    int height() const { return m_imageData.height; }

    RawImageData *getRawBuff() { return &m_imageData; }
    uint8_t *getBuff() { return m_imageData.buff; }

    bool isValid() { return m_imageData.buff != nullptr; }

    void drawToWindow(int xdest, int ydest, int width, int height, int xsrc, int ysrc, float scaleFactor);

    HDC getHandle() const { return m_hdc; }

protected:
    WindowHandle                m_windowHandle = NULL;
    RawImageData                m_imageData;
    HDC                         m_hdc = NULL;
    HBITMAP                     m_bmpMem = NULL;
    HBITMAP                     m_bmpOld = NULL;

    BITMAPINFO                  m_bmpInfo;

};

#endif // _RAW_GRAPH_DATA_H_
