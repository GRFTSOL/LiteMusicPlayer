#ifndef _SKINFILTER_CTRL_H_
#define _SKINFILTER_CTRL_H_

#pragma once

class CSkinFilterCtrl : public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinFilterCtrl();
    ~CSkinFilterCtrl();

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    void draw(CRawGraph *canvas) override;

protected:
    float m_fStartPercent;

};


#endif // _SKINFILTER_CTRL_H_
