#include "SkinTypes.h"
#include "Skin.h"
#include "SplashScreenWnd.h"


CSplashScreenWnd::CSplashScreenWnd() {
}

CSplashScreenWnd::~CSplashScreenWnd() {
}

bool CSplashScreenWnd::show(cstr_t szImageFile) {
    /*    // load image, then display it.
    CRawImage        image;
    CRawGraph        memGraph;

    if (!image.load(szImageFile))
    {
        ERR_LOG1("load Splash Image file: %s, FAILED.", szImageFile);
        return false;
    }

    int                x, y;

    x = (GetSystemMetrics(SM_CXSCREEN) - image.m_cx) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - image.m_cy) / 2;

    if (!create(nullptr, x, y, image.m_cx, image.m_cy, nullptr, DS_NOIDLEMSG))
        return false;

    IMLGraphicsBase    *canvas = getGraphics();

    assert(canvas);
    if (!canvas)
    {
        destroy();
        return false;
    }

    memGraph.create(image.m_cx, image.m_cy, canvas, 32);

    image.blt(&memGraph, 0, 0, BPM_BLEND);

    updateLayeredWindowUsingMemGraph(&memGraph);

    releaseGraphics(canvas);

    showWindow(SW_SHOW);

    setTopmost(true);
    setToolWindow(true);*/

    return true;
}
