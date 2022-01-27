// MPFloatingLyrWnd.h: interface for the CMPFloatingLyrWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
#define AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_

#pragma once

class CMPFloatingLyrWnd : public CSkinWnd
{
public:
    CMPFloatingLyrWnd();
    virtual ~CMPFloatingLyrWnd();

    int create() { return ERR_OK; }


};

extern CMPFloatingLyrWnd        g_wndFloatingLyr;

#endif // !defined(AFX_MPFloatingLyrWnd_H__45561DD4_521E_428B_9174_527AACE5C84E__INCLUDED_)
