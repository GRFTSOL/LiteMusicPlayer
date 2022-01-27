#pragma once

class CColor;
class CFontInfo;

class CGraphics
{
public:
    CGraphics();
    virtual ~CGraphics();

public:
    //
    // Win32 interface
    //
    virtual void attach(HDC hdc);
    virtual HDC detach();

    HDC getHandle() { return m_hdc; }

    virtual void setFont(CFontInfo *font) { };
    virtual bool textOut(int x, int y, cstr_t szText, int nLen) { return false; };
    virtual bool getTextExtentPoint32(cstr_t szText, int nLen, CSize *pSize) { return false; };
    virtual void setTextColor(const CColor &color) { };
    virtual void setBgColor(const CColor &color) { };

protected:
    HDC                m_hdc;

};
