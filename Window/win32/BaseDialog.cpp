#include "BaseDialog.h"


#define WM_MOUSEWHEEL                   0x020A

CBaseDialog::CBaseDialog(UINT nIDTemplate) {
    _idTemplate = nIDTemplate;

    _isModal = false;
}

CBaseDialog::~CBaseDialog() {
}

LRESULT CBaseDialog::wndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return onInitDialog();
        break;
    case ML_WM_LANGUAGE_CHANGED:
        onLanguageChanged();
        break;
    case ML_WM_USER:
        onUserMessage(wParam, lParam);
        break;
    case WM_ACTIVATE:
        {
            WORD        wActived;
            // bool        bMinimized;
            // HWND        hWndOld;

            wActived = LOWORD(wParam);

            onActivate(wActived);
        }
        break;
    case WM_CREATE:
        onCreate();
        return 0;
//     case WM_CLOSE:
//         OnClose();
//         return 0;
    case WM_COMMAND: {
            UINT uId    = LOWORD(wParam);
            onCommand(uId);
        }
        break;
    case WM_CONTEXTMENU: {
            onContexMenu(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_DESTROY: {
            HWND hWnd = m_hWnd;
            onDestroy();
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)0);
            DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    case WM_DROPFILES:
        onDropFiles((HDROP)wParam);
        break;
    case WM_HSCROLL:
        onHScroll(LOWORD(wParam), HIWORD(wParam), NULL);
        break;
    case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
        onKeyDown(wParam, lParam);
        break;
    case WM_KEYUP:
        onKeyUp(wParam, lParam);
        break;
        return 0;
    case WM_LBUTTONDOWN:
        onLButtonDown((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_LBUTTONDBLCLK:
        onLButtonDblClk((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_LBUTTONUP:
        onLButtonUp((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        break;
    case WM_MOUSEMOVE: {
            UINT nFlags = UINT(wParam);
            if (nFlags & MK_LBUTTON)
                onMouseDrag((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
            else
                onMouseMove(CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        }
        break;;
    case WM_MOUSEWHEEL: {
            onMouseWheel(short(HIWORD(wParam)), LOWORD(wParam), CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
        }
        break;
    case WM_MOVE:
        onMove(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_KILLFOCUS:
        onKillFocus();
        break;
    case WM_SETFOCUS:
        onSetFocus();
        break;
    case WM_SIZE:
        if (wParam == SIZE_RESTORED) {
            if (m_WndSizeMode != WndSizeMode_Normal) {
                m_WndSizeMode = WndSizeMode_Normal;
                onSizeModeChanged(m_WndSizeMode);
            }
            onSize(LOWORD(lParam), HIWORD(lParam));
        } else if (wParam == SIZE_MINIMIZED) {
            m_WndSizeMode = WndSizeMode_Minimized;
            onSizeModeChanged(m_WndSizeMode);
        } else if (wParam == SIZE_MAXIMIZED) {
            m_WndSizeMode = WndSizeMode_Maximized;
            onSizeModeChanged(m_WndSizeMode);
            onSize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_VSCROLL:
        onVScroll(LOWORD(wParam), HIWORD(wParam), NULL);
        break;
    case WM_TIMER:
        onTimer((UINT)wParam);
    default:
        break;
    }
    return 0;
}

int CBaseDialog::doModal(Window *pWndParent) {
    _isModal = true;

    return (int)DialogBoxParam(getAppInstance(), MAKEINTRESOURCE(_idTemplate), pWndParent ? pWndParent->getWndHandle() : NULL, (DLGPROC)baseWndProc, (LPARAM)this);
}

bool CBaseDialog::create(UINT nIDTemplate, Window *pWndParent) {
    _isModal = false;

    m_hWnd = CreateDialogParam(getAppInstance(), MAKEINTRESOURCE(nIDTemplate), pWndParent ? pWndParent->getWndHandle() : NULL, (DLGPROC)baseWndProc, (LPARAM)this);

    return m_hWnd != NULL;
}

void CBaseDialog::onCommand(UINT uID) {
    if (uID == IDOK) {
        onOK();
    } else if (uID == IDCANCEL) {
        onCancel();
    }
}

void CBaseDialog::onOK() {
    if (isModal()) {
        EndDialog(m_hWnd, IDOK);
    } else {
        destroy();
    }
}

void CBaseDialog::onCancel() {
    if (isModal()) {
        EndDialog(m_hWnd, IDCANCEL);
    } else {
        destroy();
    }
}

bool CBaseDialog::isModal() {
    return _isModal;
}

bool CBaseDialog::onInitDialog() {
    _dlgLocalization.init(m_hWnd);

    onLanguageChanged();

    if (!isChildWnd(m_hWnd)) {
        centerWindowToMonitor(m_hWnd);
    }

    return true;
}

void CBaseDialog::onLanguageChanged() {
    _dlgLocalization.toLocal();
}
