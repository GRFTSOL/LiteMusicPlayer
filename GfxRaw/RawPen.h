// RawGraph.h: interface for the CRawGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_RAW_PEN_H_)
#define _RAW_PEN_H_

#pragma once

#include "Color.h"


class CRawPen
{
public:
    CRawPen();
    virtual ~CRawPen();

    bool isValid() const { return true; }

    bool createSolidPen(int nWidth, CColor &color);

    void destroy() { }

public:
    CColor              m_clrPen;
    int                 m_nWidth;

};

#endif // !defined(_RAW_PEN_H_)
