#include "SkinTypes.h"
#include "Skin.h"
#include "SplashScreenWnd.h"


CSplashScreenWnd::CSplashScreenWnd() {
}

CSplashScreenWnd::~CSplashScreenWnd() {
}

bool CSplashScreenWnd::show(cstr_t szImageFile) {
    // load image, then display it.
    CRawImage image;
    CRawGraph memGraph;

    if (!image.load(szImageFile)) {
        ERR_LOG1("load Splash Image file: %s, FAILED.", szImageFile);
        return false;
    }

    int x, y;

    x = (GetSystemMetrics(SM_CXSCREEN) - image.m_cx) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - image.m_cy) / 2;

    if (!create(nullptr, x, y, image.m_cx, image.m_cy, nullptr, DS_NOIDLEMSG)) {
        return false;
    }

    IMLGraphicsBase *canvas = getGraphics();

    assert(canvas);
    if (!canvas) {
        destroy();
        return false;
    }

    memGraph.create(image.m_cx, image.m_cy, canvas, 32);

    image.blt(&memGraph, 0, 0, BPM_BLEND);

    updateLayeredWindowUsingMemGraph(&memGraph);

    releaseGraphics(canvas);

    showWindow(SW_SHOW);

    setTopmost(true);
    setToolWindow(true);

    return true;
}

LRESULT CSplashScreenWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    //     switch (message)
    //     {
    //     case WM_INITDIALOG:
    //         return onInitDialog();
    //         break;
    //     default:
    return Window::wndProc(message, wParam, lParam);
    //    }
}

bool CSplashScreenWnd::updateLayeredWindowUsingMemGraph(CRawGraph *canvas) {
    HDC hdc, hdcSrc;
    CRect rcWnd;
    CPoint ptSrc, ptDest;
    SIZE sizeWnd;
    uint32_t dwStyle, dwStyleNew;
    BLENDFUNCTION blend;

    hdc = GetDC(m_hWnd);

    hdcSrc = canvas->getHandle();

    // ????WS_EX_LAYERED??
    dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    dwStyleNew = dwStyle | WS_EX_LAYERED;
    if (dwStyle != dwStyleNew) {
        SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleNew);
    }

    getWindowRect(&rcWnd);

    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;

    ptSrc.x = ptSrc.y = 0;
    ptDest.x = rcWnd.left;
    ptDest.y = rcWnd.top;
    sizeWnd.cx = rcWnd.width();
    sizeWnd.cy = rcWnd.height();

    bool bRet = UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);
    //bool bRet = UpdateLayeredWindow(m_hWnd, nullptr, nullptr, nullptr, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

    ReleaseDC(m_hWnd, hdc);

    return bRet;
}
