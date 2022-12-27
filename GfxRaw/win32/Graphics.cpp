#include "MLGraphics.h"


CGraphics::CGraphics() {
    m_hdc = nullptr;
}

CGraphics::~CGraphics() {
}

void CGraphics::attach(HDC hdc) {
    m_hdc = hdc;
}

HDC CGraphics::detach() {
    HDC hdc = m_hdc;
    m_hdc = nullptr;
    return m_hdc;
}
