#pragma once

class CSkinAnimationUIObj : public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinAnimationUIObj();

    bool setProperty(cstr_t szProperty, cstr_t szValue);

    virtual void onAnimate(CSkinAnimation *pAnimation);

    int getDuration() const { return m_nDuration; }

protected:
    ListObjAnimation    m_listObjAnimation;
    int                    m_nDuration;

};
