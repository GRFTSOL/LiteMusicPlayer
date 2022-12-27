#include "RawPen.h"


//////////////////////////////////////////////////////////////////////////

CRawPen::CRawPen() {
    m_nWidth = 1;
}

CRawPen::~CRawPen() {
}

bool CRawPen::createSolidPen(int nWidth, CColor &color) {
    m_nWidth = nWidth;
    m_clrPen = color;

    return true;
}
