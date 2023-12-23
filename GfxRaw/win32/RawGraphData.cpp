#include "../GfxRaw.h"
#include "RawGraphData.h"


CRawGraphData::CRawGraphData() {
    m_hdc = nullptr;
    m_bmpMem = nullptr;
    m_bmpOld = nullptr;
}

CRawGraphData::~CRawGraphData() {
    destroy();
}

bool CRawGraphData::create(int cx, int cy, WindowHandle windowHandle, int nBitCount) {
    assert(cx > 0 && cy > 0);
    // assert(nBitCount == 32);
    m_windowHandle = windowHandle;
    HDC hdc = ::GetDC((HWND)windowHandle);

    m_bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_bmpInfo.bmiHeader.biWidth = cx;
    m_bmpInfo.bmiHeader.biHeight = cy;
    m_bmpInfo.bmiHeader.biPlanes = 1;
    m_bmpInfo.bmiHeader.biBitCount = nBitCount;
    m_bmpInfo.bmiHeader.biCompression = BI_RGB;
    m_bmpInfo.bmiHeader.biSizeImage = 0;
    m_bmpInfo.bmiHeader.biXPelsPerMeter = 0;
    m_bmpInfo.bmiHeader.biYPelsPerMeter = 0;
    m_bmpInfo.bmiHeader.biClrUsed = 0;
    m_bmpInfo.bmiHeader.biClrImportant = 0;

    m_hdc = ::CreateCompatibleDC(hdc);

    m_imageData.buff = nullptr;

    m_bmpMem = ::CreateDIBSection(
        m_hdc,
        &m_bmpInfo,
        DIB_RGB_COLORS,
        (void **)&(m_imageData.buff),
        0,
        0
        );

    m_imageData.attach(m_imageData.buff, cx, cy, nBitCount);

    m_bmpOld = (HBITMAP)::SelectObject(m_hdc, m_bmpMem);

    return true;
}

void CRawGraphData::destroy() {
    if (m_hdc) {
        // free resources
        ::SelectObject(m_hdc, m_bmpOld);
        ::DeleteObject(m_bmpMem);
        ::DeleteObject(m_hdc);
    }

    m_imageData.buff = nullptr;
}

void CRawGraphData::drawToWindow(int xdest, int ydest, int width, int height, int xsrc, int ysrc, float scaleFactor) {
	assert(0);
    // BitBlt(pTarg->getHandle(), xdest, ydest, width, height, getHandle(), xsrc, ysrc, SRCCOPY);
}
