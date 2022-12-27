#include "../WindowTypes.h"
#include "ScrollBarHandler.h"


CScrollBarHandler::CScrollBarHandler() {
    m_bDisabled = false;
    m_nOneline = 1;
}

CScrollBarHandler::~CScrollBarHandler() {

}

void CScrollBarHandler::init(Window *pWnd, int nBar) {
    assert(pWnd);

    m_nScrollBar = nBar;
}

void CScrollBarHandler::setScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw) {

}

int CScrollBarHandler::setScrollPos(int nPos, bool bRedraw) {
    return nPos;
}

int CScrollBarHandler::getID() const {
    return 0;
}

int CScrollBarHandler::getScrollPos() const {
    return 0;
}

int CScrollBarHandler::getMax() const {
    return 0;
}

void CScrollBarHandler::disableScrollBar() {
}

bool CScrollBarHandler::handleScrollCode(uint32_t nSBCode, int nPos) {
    return false;
}
