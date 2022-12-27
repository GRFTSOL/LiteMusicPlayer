#pragma once

class CSkinTabButton : public CSkinToolbar {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinToolbar)
public:
    CSkinTabButton();
    virtual ~CSkinTabButton();

    void draw(CRawGraph *canvas) override;

    virtual void onCreate() override;

    virtual bool onMenuKey(uint32_t nChar, uint32_t nFlags) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

protected:
    virtual void drawButton(CRawGraph *canvas, int nButton, BT_DRAW_STATE btDrawState, int x, int y, int nHeight, int nBtImageLeft) override;

protected:
    int                         m_nSperatorLineWidth;
    int                         m_nButtonFaceWidth, m_nButtonBorderWidth;

};
