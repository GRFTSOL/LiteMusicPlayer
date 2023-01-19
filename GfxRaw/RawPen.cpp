#include "RawPen.h"


//////////////////////////////////////////////////////////////////////////

CRawPen::CRawPen(int width, COLORREF clr) : m_nWidth(width), m_clrPen(clr) {
}

CRawPen::~CRawPen() {
}

bool CRawPen::createSolidPen(int width, const CColor &color) {
    m_nWidth = width;
    m_clrPen = color;

    return true;
}
