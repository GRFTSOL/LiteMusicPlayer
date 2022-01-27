#ifndef _RAW_GRAPH_DATA_H_
#define _RAW_GRAPH_DATA_H_

#pragma once

class CRawGraphData
{
public:
    CRawGraphData();
    virtual ~CRawGraphData();

    virtual bool create(int cx, int cy, IMLGraphicsBase *canvas, int nBitCount = 32);

    virtual void destroy();

    int width() const { return m_imageData.width; }
    int height() const { return m_imageData.height; }

    RawImageData *getRawBuff() { return &m_imageData; }
    uint8_t *getBuff() { return m_imageData.buff; }

    bool isValid() { return m_imageData.buff != nullptr; }

    virtual void drawToWindow(int xdest, int ydest, int width, int height);
    virtual void drawToWindowStretch(int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc);

public:
    // For win32 only
    GdkPixbuf *getHandle() const { return m_pGdkPixBuf; }

protected:
    RawImageData    m_imageData;
    GdkPixbuf        *m_pGdkPixBuf;

};

#endif // _RAW_GRAPH_DATA_H_
