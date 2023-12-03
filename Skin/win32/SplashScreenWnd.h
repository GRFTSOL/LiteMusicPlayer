#if !defined(_SPLASHSCREENWND_H_)
#define _SPLASHSCREENWND_H_

#pragma once

class CSplashScreenWnd : public Window {
public:
    CSplashScreenWnd();
    virtual ~CSplashScreenWnd();

public:
    bool show(cstr_t szImageFile);

protected:
    virtual LRESULT wndProc(uint32_t message, WPARAM wParam, LPARAM lParam);

    bool updateLayeredWindowUsingMemGraph(CRawGraph *canvas);

};

#endif // !defined(_SPLASHSCREENWND_H_)
