// MediaAlbumArtCtrl.h: interface for the CMediaAlbumArtCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_)
#define AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMediaAlbumArtCtrl : public CUIObject
{
    UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
    CMediaAlbumArtCtrl();
    virtual ~CMediaAlbumArtCtrl();

    void onCreate();

    virtual bool onLButtonUp( uint32_t nFlags, CPoint point );

    virtual bool onLButtonDown(uint32_t nFlags, CPoint point);

    void onTimer(int nId);

    void onSize(int cx, int cy);

    void draw(CRawGraph *canvas);

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;
    void enumProperties(CUIObjProperties &listProperties);

protected:
    void updateAlbumArt();

    void createAlbumArtImage(RawImageData *pImgSrc);

    void resizeAlbumArt();

    bool isMaskImageUsed() { return m_frmMask.isValid(); }

    string                m_strImgFile;
    CSFImage            m_img;

    // no album art mask image
    CSFImage            m_imgOrg;    // save original album art for resize.

    // album art frame mask image
    string                m_strFrmMask;
    CSFImage            m_frmMask;
    CRect                m_rcFrameMaskPos;

    CColor                m_clrBg;
    CMLBrush            m_brBg;

};

#endif // !defined(AFX_MEDIAALBUMARTCTRL_H__525B5E41_E06C_4FE2_96B8_F36CA4C6C807__INCLUDED_)
