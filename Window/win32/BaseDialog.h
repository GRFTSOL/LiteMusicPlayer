#pragma once

#include "DialogLocalization.h"


class CBaseDialog : public Window {
public:
    virtual bool create(UINT nIDTemplate, Window *pWndParent);

    int doModal(Window *pWndParent);
    bool isModal();

    virtual void onCancel();
    virtual void onOK();

    void onCommand(uint32_t id) override;
    virtual bool onInitDialog();
    void onLanguageChanged() override;

    LRESULT wndProc(UINT message, WPARAM wParam, LPARAM lParam) override;

public:
    CBaseDialog(UINT nIDTemplate);
    virtual ~CBaseDialog();

    bool                        _isModal = true;

    UINT                        _idTemplate = 0;

    DialogLocalization          _dlgLocalization;

};
