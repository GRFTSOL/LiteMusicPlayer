// SplashScreenWnd.h: interface for the CSplashScreenWnd class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CSplashScreenWnd : public Window  
{
public:
    CSplashScreenWnd();
    virtual ~CSplashScreenWnd();

public:
    bool show(cstr_t szImageFile);

};

