#pragma once

class CWidgetGradientCtrl : public CWidgetCtrlBase {
public:
    enum    { COUNT = 3 };

    CWidgetGradientCtrl();
    ~CWidgetGradientCtrl();

    void setColor(CColor clr[COUNT]);
    void getColor(CColor clr[COUNT]);

protected:
    CRawPen                     m_pen;
    CColor                      m_vColors[COUNT];

};
