#pragma once

#ifndef MPlayerUI_SkinRateCtrl_h
#define MPlayerUI_SkinRateCtrl_h


class CSkinRateCtrl : public CUIObject {
public:
    CSkinRateCtrl();
    virtual ~CSkinRateCtrl();

    cstr_t getClassName() override;

    bool onLButtonUp(uint32_t nFlags, CPoint point) override;
    bool onLButtonDown(uint32_t nFlags, CPoint point) override;
    void draw(CRawGraph *canvas) override;
    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void setRating(int nRating);
    int getRating() { return m_nRating; }

public:
    static cstr_t className() { return ms_szClassName; }
    bool isKindOf(cstr_t szClassName) override;

protected:
    string                      m_strBmpFile;
    CSFImage                    m_img;
    int                         m_nRateStarWidth;

    int                         m_nRating;          // 0 ~ 5
    int                         m_nRatingMax;

    bool                        m_bLBtDown;

    static cstr_t               ms_szClassName;

};

#endif // !defined(MPlayerUI_SkinRateCtrl_h)
