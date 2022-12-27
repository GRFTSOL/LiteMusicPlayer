// MPSkinVis.h: interface for the CMPSkinVis class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_)
#define AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../MPlayerEngine/IMPlayer.h"

#ifdef SKIN_EDITOR
class IEventHandler
{
};
#endif

class CMPSkinVis : public CUIObject, public IEventHandler
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    enum VisMode
    {
        VM_OSILLOSCOPE,
        VM_SPECTRUM,
    };

    CMPSkinVis();
    virtual ~CMPSkinVis();

#ifndef SKIN_EDITOR
    virtual void onCreate() override;

    virtual void onEvent(const IEvent *pEvent) override;
#endif

    virtual bool onLButtonUp( uint32_t nFlags, CPoint point ) override;

    virtual void draw(CRawGraph *canvas) override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

protected:
    virtual void drawOsilloscope(CRawGraph *canvas, VisParam *visParam);

    virtual void drawSpectrum(CRawGraph *canvas, VisParam *visParam);

protected:
    VisMode                m_visMode;
    CColor                m_clrFg;
    CRawPen                m_pen;

    string                m_strBgFile, m_strSpectrumColFile;;
    CSFImage            m_imgBg, m_imgSpectrumCol;
    int                    m_nSpectrumUnitHeight;
    int                    m_nSpectrumColSpace;
    int                    m_nLeftMargin, m_nRightMargin, m_nTopMargin, m_nBottomMargin;

    VisParam            *m_visParamCur;

    // vector<int>            m_vSpectrumDropDownMax, m_vSpectrumMax;
    struct SpetrumCol
    {
        short        nMax;
        short        nDropMax;
        float        nDropSpeed;
    };
    vector<SpetrumCol>    m_vSpectrumDrop;

};

#endif // !defined(AFX_SKINVIS_H__A9BE151E_BD76_49B7_AD07_37D7C798122B__INCLUDED_)
