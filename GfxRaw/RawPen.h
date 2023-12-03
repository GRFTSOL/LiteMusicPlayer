#if !defined(_RAW_PEN_H_)
#define _RAW_PEN_H_

#pragma once

#include "Color.h"


class CRawPen {
public:
    CRawPen(int width = 1, COLORREF clr = RGB(0, 0, 0));
    virtual ~CRawPen();

    bool isValid() const { return true; }

    bool createSolidPen(int width, const CColor &color);

    void destroy() { }

public:
    CColor                      m_clrPen;
    int                         m_nWidth;

};

#endif // !defined(_RAW_PEN_H_)
