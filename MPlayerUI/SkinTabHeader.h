// SkinTabHeader.h: interface for the CSkinTabHeader class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SKIN_TAB_HEADER_INC_
#define _SKIN_TAB_HEADER_INC_

#pragma once

#include "UIObject.h"


//
//    TabHeader image must be in following alignment
//           // [ 0 [ 1 [ 2 ] 3 ] 4 ]
//        2 is active button 
//
class CSkinTabHeader : public CUIObject  
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CSkinTabHeader();
    virtual ~CSkinTabHeader();

    struct Button
    {
        int            nID;
        int            nIDAssociatedObj;
        int            nAutoUniID;
        string        strText;
        string        strTooltip;
        bool        bTempToolTip;
        string        strIDAssociatedObj;

        int            nWidth;
        int            nImageWidth;
        int            nImageStartPos;

        Button();

        void fromXML(CSkinTabHeader *pTabHeader, SXNode *pNode);
#ifdef _SKIN_EDITOR_
        void toXML(CSkinTabHeader *pTabHeader, CXMLWriter &xml);
#endif // _SKIN_EDITOR_

    };

    friend struct Button;

    typedef vector<Button>        vec_buttons;

public:
    void onCreate();

    bool onMouseMove(uint32_t nFlags, CPoint point);
    bool onLButtonUp(uint32_t nFlags, CPoint point);
    bool onLButtonDown(uint32_t nFlags, CPoint point);
    bool onLButtonDblClk(uint32_t nFlags, CPoint point);

    void onSize();

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue);
#ifdef _SKIN_EDITOR_
    void enumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

    virtual int fromXML(SXNode *pXmlNode);

#ifdef _SKIN_EDITOR_
    // write Button in XML
    virtual bool hasXMLChild() { return true; }
    virtual void onToXMLChild(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

protected:
    int buttonHitTest(CPoint pt);

    void invalidateButton(int nButton);

    void drawTabButton(CRawGraph *canvas, Button &bt, CRect &rcButton, bool bActiveTabButton);

protected:
    int                    m_nButtonFaceSize;
    int                    m_nButtonBorderSize;
    int                    m_nTabHeight;

    int                    m_nActiveButtonIncSize;

    //
    // Image order:  Inactive, active, active-mask
    //
    enum  TabImagePos
    {
        TIP_INACTIVE,
        TIP_ACTIVE,
        TIP_ACTIVE_MASK,
    };
    string                m_strImageFile;
    CSFImage            m_img;

    bool                m_bEnableHover;
    bool                m_bLBtDown;
    bool                m_bHover;

    vec_buttons            m_vButtons;
    int                    m_nActiveButton;
    int                    m_nHoverButton;

    CSkinFontProperty    m_font;

};

#endif // !_SKIN_TAB_HEADER_INC_
