#include "WidgetGradientCtrl.h"
#include "MPlayerApp.h"
#include "LyricShowObj.h"


CWidgetGradientCtrl::CWidgetGradientCtrl() {
    CColor clr(0);
    m_pen.CreateSolidPen(1, clr);

    m_vColors[0].set(RGB(0, 255, 0));
    m_vColors[1].set(RGB(255, 0, 0));
    m_vColors[2].set(RGB(0, 0, 255));
}

CWidgetGradientCtrl::~CWidgetGradientCtrl() {
}

void CWidgetGradientCtrl::setColor(CColor clr[COUNT]) {
    for (int i = 0; i < COUNT; i++) {
        m_vColors[i] = clr[i];
    }
}

void CWidgetGradientCtrl::getColor(CColor clr[COUNT]) {
    for (int i = 0; i < COUNT; i++) {
        clr[i] = m_vColors[i];
    }
}
