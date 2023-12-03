#include "GfxRaw.h"
#include "RawGraphData.h"


CRawGraphData::CRawGraphData() {
    m_pGdkPixBuf = nullptr;
}

CRawGraphData::~CRawGraphData() {
    destroy();
}

bool CRawGraphData::create(int cx, int cy, IMLGraphicsBase *canvas, int nBitCount) {
    assert(nBitCount == 32);

    if (!m_imageData.createReverse(cx, cy, nBitCount)) {
        return false;
    }

    m_pGdkPixBuf = gdk_pixbuf_new_from_data(m_imageData.buff,
        GDK_COLORSPACE_RGB,
        nBitCount == 32,
        8,
        cx,
        cy,
        m_imageData.stride,
        nullptr,
        nullptr);

    return m_pGdkPixBuf != nullptr;
}

void CRawGraphData::destroy() {
    gdk_pixbuf_unref(m_pGdkPixBuf);

    m_imageData.free();
}

void CRawGraphData::drawToWindow(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc) {
    CGraphics *canvas = (CGraphics *)pTarg;
    gdk_pixbuf_render_to_drawable(m_pGdkPixBuf, canvas->GetDrawable(), canvas->GetGC(), xsrc, ysrc, xdest, ydest, width, height, GDK_RGB_DITHER_NORMAL, 0, 0);
}

void CRawGraphData::drawToWindowStretch(IMLGraphicsBase *pTarg, int xdest, int ydest, int width, int height, int xsrc, int ysrc, int widthsrc, int heightsrc) {
    CGraphics *canvas = (CGraphics *)pTarg;
    gdk_pixbuf_render_to_drawable(m_pGdkPixBuf, canvas->GetDrawable(), canvas->GetGC(), xsrc, ysrc, xdest, ydest, width, height, GDK_RGB_DITHER_NORMAL, 0, 0);
    // stretchBlt(pTarg->getHandle(), xdest, ydest, width, height, getHandle(), xsrc, ysrc, widthsrc, heightsrc, SRCCOPY);
}
