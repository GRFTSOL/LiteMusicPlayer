#include "../SkinTypes.h"
#include "../SkinWnd.h"
#include <Dwmapi.h>


typedef HRESULT (WINAPI *FUNC_DwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset);

bool extendIntoWholeClient(HWND hwnd, bool bIntoWholeClient) {
    // set margins, extending to whole client
    static FUNC_DwmExtendFrameIntoClientArea pDwmExtendFrameIntoClientArea = nullptr;
    if (!pDwmExtendFrameIntoClientArea) {
        HMODULE hModule = LoadLibrary("Dwmapi.dll");
        if (hModule) {
            pDwmExtendFrameIntoClientArea = (FUNC_DwmExtendFrameIntoClientArea)GetProcAddress(hModule, "DwmExtendFrameIntoClientArea");
        }
    }

    if (!pDwmExtendFrameIntoClientArea) {
        return false;
    }

    MARGINS margins = {-1};
    HRESULT hr = S_OK;

    if (!bIntoWholeClient) {
        memset(&margins, 0, sizeof(margins));
    }

    hr = pDwmExtendFrameIntoClientArea(hwnd,&margins);
    if (!SUCCEEDED(hr)) {
        DBG_LOG1("Failed to apply Aero glass: %s", (cstr_t)Error2Str(hr));
        return false;
    }

    return true;
}


bool CSkinWnd::invalidateRect(const CRect *lpRect, bool bErase) {
    if (!isVisible() || isIconic()) {
        return false;
    }

    if (m_nInRedrawUpdate > 0) {
        m_needRedraw = true;
        return false;
    }

    if (!m_bTranslucencyLayered) {
        return Window::invalidateRect(lpRect, bErase);
    }

    invalidateRectOfLayeredWindow(lpRect);

    return true;
}

void CSkinWnd::invalidateRectOfLayeredWindow(const CRect* lpRect) {
    assert(m_bTranslucencyLayered);
    CRawGraph *memCanvas;

    memCanvas = getMemGraphics();
    CRawGraph::CClipBoxAutoRecovery autoCBR(memCanvas);
    if (lpRect) {
        memCanvas->setClipBoundBox(*lpRect);
    }

    // draw every ui objects on back buffer one by one
    m_rootConainter.draw(memCanvas);

    updateLayeredWindowUsingMemGraph(memCanvas);
}
