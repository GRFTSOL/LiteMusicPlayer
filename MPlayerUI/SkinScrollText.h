
#pragma once


class CSkinScrollText : public CSkinStaticText  
{
    UIOBJECT_CLASS_NAME_DECLARE(CSkinStaticText)
public:
    CSkinScrollText();
    virtual ~CSkinScrollText();

    void draw(CRawGraph *canvas);

//     bool setProperty(cstr_t szProperty, cstr_t szValue);
//     void enumProperties(CUIObjProperties &listProperties);

//    void onCreate();

    bool onLButtonUp(uint32_t nFlags, CPoint point);
    bool onLButtonDown(uint32_t nFlags, CPoint point);

    void onTimer(int nId);

protected:
    int            m_nTimerIDScroll;
    int            m_nPosScroll;
    bool        m_bToLeft;
    int            m_nWidthText;

};
