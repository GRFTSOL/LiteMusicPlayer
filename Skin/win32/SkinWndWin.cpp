#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
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
    m_skinToolTip.onPaint(memCanvas);

    updateLayeredWindowUsingMemGraph(memCanvas);
}

/*

void testUpdateLayeredWindow(HWND hWnd, HBITMAP hbmp, int nBmpWidth, int nBmpHeight)
{
    HDC        hdc, hdcSrc;
    HBITMAP    hbmpOld;
    BLENDFUNCTION    blend;
    CPoint    ptSrc, ptDest;
    SIZE    sizeWnd;
    CRect    rcWnd;
    uint32_t    dwStyle;

    hdc = GetDC(hWnd);

    // ????WS_EX_LAYERED??
    dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    dwStyle |= WS_EX_LAYERED;
    // SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle & ~ (WS_EX_LAYERED));
    SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

    hdcSrc = CreateCompatibleDC(hdc);

    hbmpOld = (HBITMAP)SelectObject(hdcSrc, hbmp);

#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA                0x01
#endif

    getWindowRect(hWnd, &rcWnd);

    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;

    ptSrc.x = ptSrc.y = 0;
    ptDest.x = rcWnd.left;
    ptDest.y = rcWnd.top;
    sizeWnd.cx = nBmpWidth;
    sizeWnd.cy = nBmpHeight;

    bool bRet = UpdateLayeredWindow(hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcSrc, hbmpOld);

    ReleaseDC(hWnd, hdc);
    DeleteDC(hdcSrc);
}

HBITMAP load32Bitbmp(cstr_t szImage, int &nWidth, int &nHeight)
{
    CImgPngFile        png;
    CImageData        imgData;

    png.setImgData(&imgData);
    if (!png.open(szImage))
        return nullptr;

    nWidth = imgData.GetWidth();
    nHeight = imgData.getHeight();

    return imgData.ToHBitmap(nullptr);
}*/

LRESULT CSkinWnd::wndProc(uint32_t message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NOTIFY) {
        NMHDR *pnmh = (NMHDR *)lParam;

        if (pnmh->code == TTN_GETDISPINFO) {
            NMTTDISPINFO *lpnmtdi = (NMTTDISPINFO *)lParam;

            if ((lpnmtdi->uFlags & TTF_IDISHWND) != TTF_IDISHWND) {
                int nId;

                nId = lpnmtdi->hdr.idFrom;

                CUIObject *pObj;

                pObj = getUIObjectById(nId);
                if (pObj) {
                    strcpy_safe(lpnmtdi->szText, CountOf(lpnmtdi->szText), _TL(pObj->m_strTooltip.c_str()));
                    lpnmtdi->lpszText = lpnmtdi->szText;
                    lpnmtdi->hinst = nullptr;
                }
            }
        }
    } else if (message == WM_GETDLGCODE) {
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
