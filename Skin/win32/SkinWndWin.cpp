﻿#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT	0x0500
#include <windows.h>
#include <Dwmapi.h>
#include "../Skin.h"


bool CSkinWnd::invalidateRect(const CRect *lpRect, bool bErase) {
    if (!isVisible() || isIconic()) {
        return false;
    }

    if (m_nInRedrawUpdate > 0) {
        m_needRedraw = true;
        return false;
    }

    return Window::invalidateRect(lpRect, bErase);
}

void CSkinWnd::invalidateRectOfLayeredWindow(const CRect* lpRect) {
    assert(m_bTranslucencyLayered);
    auto memCanvas = getMemGraphics();
    CRawGraph::CClipBoxAutoRecovery autoCBR(memCanvas);
    if (lpRect) {
        memCanvas->setClipBoundBox(*lpRect);
    }

    // draw every ui objects on back buffer one by one
    m_rootConainter.draw(memCanvas);
    m_skinToolTip.onPaint(memCanvas);

    updateLayeredWindowUsingMemGraph(memCanvas);
}

LRESULT CSkinWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_GETDLGCODE) {
        return DLGC_WANTMESSAGE;
    } else if (message == WM_CAPTURECHANGED) {
        m_pUIObjCapMouse = nullptr;
    } else if (message == WM_SYSCHAR) {
        if (m_rootConainter.onMenuKey(wParam, lParam)) {
            return 0;
        }
    }
    //     else if (message == WM_SYSCOMMAND)
    //     {
    //
    //         return 0;
    //     }

    switch (message) {
    case WM_NCCALCSIZE:
        {
            // reset its client area.
            //            fCalcValidRects = (bool) wParam;        // valid area flag
            //            lpncsp = (LPNCCALCSIZE_PARAMS) lParam;    // size calculation data    or
            //            OnNcCalcSize((bool)wParam, (LPNCCALCSIZE_PARAMS)lParam);
            return 0;
        }
        //     case WM_CAPTURECHANGED:
        //         if (m_pUIObjCapMouse)
        //             releaseCaptureMouse(m_pUIObjCapMouse);
        //         break;
    case WM_DESTROY:
        {
            // after process WM_DESTORY, delete this.
            onDestroy();
            if (m_hWnd) {
                SetWindowLongPtr(m_hWnd, GWLP_USERDATA, 0);
                DefWindowProc(m_hWnd, message, wParam, lParam);
                m_hWnd = nullptr;
            }
            if (m_bFreeOnDestory) {
                delete this;
            }
        }
        return 0;
    case WM_ERASEBKGND:
        return 0;
    case WM_PAINT:
        {
            if (m_bTranslucencyLayered) {
                CRect rcClip;
                if (::GetUpdateRect(m_hWnd, &rcClip, false)) {
                    invalidateRectOfLayeredWindow(&rcClip);
                }

                // updates the position, size, shape, content, and translucency of a layered window
                return DefWindowProc(m_hWnd, message, wParam, lParam);
            } else {
                CRect rcClip;
                if (!::GetUpdateRect(m_hWnd, &rcClip, false)) {
                    return 0;
                }

                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(m_hWnd, &ps);

                auto canvas = getMemGraphics();
                onPaint(canvas, &rcClip);

                EndPaint(m_hWnd, &ps);
            }
        }
        return 0;
    }

    return Window::wndProc(message, wParam, lParam);
}
