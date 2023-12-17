#include "VISDemo.h"


CVISDemo::CVISDemo() {
    OBJ_REFERENCE_INIT
    m_pPlayer = nullptr;
}

CVISDemo::~CVISDemo() {

}

ResultCode CVISDemo::init(IMPlayer *pPlayer) {
    m_pPlayer = pPlayer;
    m_pPlayer->addRef();

    createWnd();

    return ERR_OK;
}

ResultCode CVISDemo::quit() {
    assert(m_pPlayer);
    m_pPlayer->release();

    m_pPlayer = nullptr;

    destroyWindow(m_hWnd);

    return ERR_OK;
}

HDC memDC; // memory device context
HBITMAP    memBM,  // memory bitmap (for memDC)
oldBM; // old bitmap (from memDC)

int width = 488,height = 256 * 2 + 30;

int nMode = 4;

int CVISDemo::render(VisParam *visParam) {
    if (nMode == 2) {
        return render2(visParam);
    } else if (nMode == 3) {
        return render3(visParam);
    } else if (nMode == 4) {
        return render4(visParam);
    }
    int x, y;
    // clear background
    rectangle(memDC,0,0,256,32);
    // draw VU meter
    for (y = 0; y < 2; y ++) {
        int last=visParam->waveformData[y][0];
        int total=0;
        for (x = 1; x < VIS_N_SPTR_SAMPLE; x ++) {
            total += abs(last - visParam->waveformData[y][x]);
            last = visParam->waveformData[y][x];
        }
        total /= 288;
        if (total > 127) total = 127;
        if (y) rectangle(memDC,128,0,128+total,32);
        else rectangle(memDC,128-total,0,128,32);
    }
    { // copy doublebuffer to window
        SetLastError(0);
        HDC hdc = GetDC(m_hWnd);
        BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
        ReleaseDC(m_hWnd,hdc);
    }

    return ERR_OK;
}

int CVISDemo::render2(VisParam *visParam) {
    int x, y;
    // clear background
    rectangle(memDC,0,0,288,256);
    // draw analyser
    for (y = 0; y < visParam->nChannels; y ++) {
        for (x = 0; x < 288; x ++) {
            MoveToEx(memDC,x,(y*256+256)>>(visParam->nChannels-1),nullptr);
            LineTo(memDC,x,(y*256 + 256 - visParam->spectrumData[y][x])>>(visParam->nChannels-1));
        }
    }
    { // copy doublebuffer to window
        SetLastError(0);
        HDC hdc = GetDC(m_hWnd);
        BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
        ReleaseDC(m_hWnd,hdc);
    }
    return 0;
}

int CVISDemo::render4(VisParam *visParam) {
    int y;
    static int pos = 0;
    // clear background
    //if (pos == 0)
    //    rectangle(memDC,0,0,width,height);
    pos++;
    if (pos > width) {
        pos = 0;
    }

    y = 0;
    for (int i = 0; i < visParam->nChannels; i++) {
        for (int y1 = 0; y1 < 288; y1 ++) {
            uint8_t b = visParam->spectrumData[i][y1] * 2;
            setPixel(memDC, pos, y + y1, RGB(b, b, b));
            // MoveToEx(memDC,x,(y*256+256)>>(visParam->nChannels-1),nullptr);
            // LineTo(memDC,x,(y*256 + 256 - visParam->spectrumData[y][x])>>(visParam->nChannels-1));
        }
        y += 257;
    }
    { // copy doublebuffer to window
        SetLastError(0);
        HDC hdc = GetDC(m_hWnd);
        BitBlt(hdc,0,0,width,height,memDC,0,0,SRCCOPY);
        ReleaseDC(m_hWnd,hdc);
    }
    return 0;
}

int CVISDemo::render3(VisParam *visParam) {
    int x, y;
    // clear background
    rectangle(memDC,0,0,288,256);
    // draw oscilliscope
    for (y = 0; y < visParam->nChannels; y ++) {
        MoveToEx(memDC,0,(y*256)>>(visParam->nChannels-1),nullptr);
        for (x = 0; x < 288; x ++) {
            LineTo(memDC,x,(y*256 + visParam->waveformData[y][x]^128)>>(visParam->nChannels-1));
        }
    }
    { // copy doublebuffer to window
        SetLastError(0);
        HDC hdc = GetDC(m_hWnd);
        BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
        ReleaseDC(m_hWnd,hdc);
    }
    return 0;
}

// window procedure for our window
LRESULT CALLBACK wndProc(HWND hwnd, uint32_t message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: return 0;
        case WM_ERASEBKGND: return 0;
    case WM_PAINT:
        {
            // update from doublebuffer
            PAINTSTRUCT ps;
            CRect r;
            HDC hdc = BeginPaint(hwnd,&ps);
            getClientRect(hwnd,&r);
            BitBlt(hdc,0,0,r.right,r.bottom,memDC,0,0,SRCCOPY);
            {
                CRect x={r.left+width, r.top, r.right, r.bottom};
                CRect y={r.left, r.top+height, r.right, r.bottom};
                FillRect(hdc, &x, (HBRUSH)GetStockObject(WHITE_BRUSH));
                FillRect(hdc, &y, (HBRUSH)GetStockObject(WHITE_BRUSH));
            }
            EndPaint(hwnd,&ps);
        }
        return 0;
        case WM_DESTROY: postQuitMessage(0); return 0;
    case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
    case WM_KEYUP:
        return 0;
    case WM_LBUTTONDOWN:
        nMode++;
        if (nMode > 4) {
            nMode = 1;
        }
        return 0;
        //    case WM_WINDOWPOSCHANGING:
        //         {    // get config_x and config_y for configuration
        //             CRect r;
        //             getWindowRect(myWindowState.me,&r);
        //             config_x = r.left;
        //             config_y = r.top;
        //         }
        //        return 0;
    }
    return defWindowProc(hwnd,message,wParam,lParam);
}


bool CVISDemo::createWnd() {

    {    // register our window class
        WNDCLASS wc;
        memset(&wc,0,sizeof(wc));
        wc.lpfnWndProc = wndProc; // our window procedure
        wc.hInstance = getAppInstance(); // hInstance of DLL
        wc.lpszClassName = "Demo Vis"; // our window class name

        if (!RegisterClass(&wc)) {
            messageBox(nullptr, "Error registering window class", "Message", MB_OK);
            return false;
        }
    }

    uint32_t styles = WS_VISIBLE|WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;


    m_hWnd = CreateWindowEx(
        0,    // these exstyles put a nice small frame,
        // but also a button in the taskbar
        "Demo Vis",                            // our window class name
        nullptr,                // no title, we're a child
        styles,                                   // do not make the window visible
        0,0,                    // screen position (read from config)
        width,height,                        // width & height of window (need to adjust client area later)
        nullptr,                // parent window (winamp main window)
        nullptr,                                // no menu
        getAppInstance(),                // hInstance of DLL
        0); // no window creation data

    if (!m_hWnd) {
        messageBox(nullptr, "Error creating window", "Message", MB_OK);
        return false;
    }

    SetWindowLong(m_hWnd,GWL_USERDATA,(LONG)this); // set our user data to a "this" pointer

    /*    {    // adjust size of window to make the client area exactly width x height
        CRect r;
        getClientRect(hMainWnd,&r);
        setWindowPos(hMainWnd,0,0,0,width*2-r.right,height*2-r.bottom,SWP_NOMOVE|SWP_NOZORDER);
    }*/

    // create our doublebuffer
    memDC = CreateCompatibleDC(nullptr);
    memBM = CreateCompatibleBitmap(memDC,width,height);
    oldBM = (HBITMAP)SelectObject(memDC,memBM);

    {
        CRect r={0,0,width,height};
        FillRect(memDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
    }

    // show the window
    showWindow(m_hWnd, SW_SHOWNORMAL);
    return 0;
}
