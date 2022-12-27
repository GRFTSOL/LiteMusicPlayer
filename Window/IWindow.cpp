//
//  IWindow.cpp
//  Mp3Player
//
//  Created by HongyongXiao on 2021/11/13.
//

#include "IWindow.h"
#include "../GfxRaw/GfxRaw.h"


IWindow::IWindow() {
    m_handleHolder = nullptr;
    m_pmemGraph = nullptr;
}


IWindow::~IWindow() {
    if (m_pmemGraph) {
        delete m_pmemGraph;
        m_pmemGraph = nullptr;
    }
}

void IWindow::onCreate() {
    onLanguageChanged();
}

void IWindow::onResized(int width, int height) {
    if (m_wndSize.cx == width && m_wndSize.cy == height && m_pmemGraph) {
        return;
    }
    m_wndSize.cx = width;
    m_wndSize.cy = height;

    recreateMemGraphics();

    onSize(width, height);
}
//
//void IWindow::onActivate(bool isActive) {
////    if (_windowContext) {
////        _windowContext->activate(isActive);
////    }
////    fIsActive = isActive;
//}


CRawGraph *IWindow::getMemGraphics() {
    //    assert(m_pmemGraph);
    if (!m_pmemGraph) {
        recreateMemGraphics();
    }

    m_pmemGraph->resetClipBoundBox(CRect(0, 0, m_pmemGraph->width(), m_pmemGraph->height()));
    m_pmemGraph->setOrigin(CPoint(0, 0));

    return m_pmemGraph;
}

bool IWindow::recreateMemGraphics() {
    if (m_pmemGraph) {
        delete m_pmemGraph;
    }

    CRawGraph *memCanvas = new CRawGraph();
    memCanvas->create(m_wndSize.cx, m_wndSize.cy, m_handleHolder);
    m_pmemGraph = memCanvas;

    return true;
}
