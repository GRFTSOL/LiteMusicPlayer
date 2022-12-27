#if !defined(_SPLASHSCREENWND_H_)
#define _SPLASHSCREENWND_H_

#pragma once

class CSplashScreenWnd : public Window {
public:
    CSplashScreenWnd();
    virtual ~CSplashScreenWnd();

public:
    bool show(cstr_t szImageFile);

};

#endif // !defined(_SPLASHSCREENWND_H_)
