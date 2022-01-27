/////////////////////////////////////////////////////////////
// My Library like mfc
#ifndef _OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED
#define _OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED

////////////////////////////////////////////////////////////
//
bool isTopmostWindow(HWND hWnd);

//
// remove it's WS_EX_APPWINDOW AND ADD WS_EX_TOOLWINDOW
bool showAsToolWindow(HWND hWnd);

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
bool showAsAppWindow(HWND hWnd);

bool isToolWindow(HWND hWnd);

void topmostWindow(HWND hwnd, bool bTopmost);

// PURPOSE:
//      activate the windows
void activateWindow(HWND hWnd);

bool isChildWnd(HWND hWnd);

void setParentByForce(HWND hWndChild, HWND hWndParent);

bool moveWindowSafely(HWND hWnd, int X, int Y, int nWidth, int nHeight, bool bRepaint);

bool enableDlgItem(HWND hWnd, int nIDItem, bool bEnable);
bool showDlgItem(HWND hWnd, int nIDItem, int nCmdShow);
#define SetDlgItemFocus(hWnd, nIDItem)    ::setFocus(::getDlgItem(hWnd, nIDItem))

bool browserForFolder(HWND hWnd, cstr_t szTitle, char * szPath, cstr_t szRootFoler = nullptr);

// COMMENT:
//        判断系统是否支持半透明窗口
//        只有Windows2000或以上才支持
bool isLayeredWndSupported();

//    COMMENT:
//        sets the opacity and transparency color key of a layered window
//    INPUT:
//        crKey,        specifies the color key
//        bAlpha,        value for the blend function, When bAlpha is 0, 
//                    the window is completely transparent. 
//                    When bAlpha is 255, the window is opaque. 
//        dwFlags        Specifies an action to take. This parameter can be one or 
//                    more of the following values. Value Meaning 
//                    LWA_COLORKEY Use crKey as the transparency color.  
//                    LWA_ALPHA Use bAlpha to determine the opacity of the layered window 
bool setLayeredWindow(HWND hWnd, COLORREF crKey, uint8_t bAlpha, uint32_t dwFlags);

void unSetLayeredWindow(HWND hWnd);

/*******************************************************************/


#endif  //_OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED