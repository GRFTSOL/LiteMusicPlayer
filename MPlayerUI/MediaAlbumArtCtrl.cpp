#include "MPlayerAppBase.h"
#include "MediaAlbumArtCtrl.h"


RawImageData *convertTo32BppRawImage(RawImageData *src);

RawImageData *createScaleImage(RawImageData *pImgSrc, int wDst, int hDst) {
    assert(pImgSrc);
    CRect rc;
    int w, h;

    if (!pImgSrc || pImgSrc->height == 0 || pImgSrc->width == 0) {
        return nullptr;
    }

    w = pImgSrc->width * hDst / pImgSrc->height;
    if (w < wDst) {
        // fill left and right
        h = hDst;
    } else {
        // fill top and bottom
        h = pImgSrc->height * wDst / pImgSrc->width;
        w = wDst;
    }

    CRawImage imgSrc;
    CRawImage imgDst;
    RawImageData *pImgDst;

    pImgDst = new RawImageData;
    if (!pImgDst->create(w, h, pImgSrc->bitCount)) {
        return nullptr;
    }

    imgSrc.attach(pImgSrc);
    imgDst.attach(pImgDst);

    imgSrc.stretchBlt(&imgDst, 0, 0, w, h, BPM_COPY | BPM_BILINEAR);

    imgSrc.detach();
    imgDst.detach();

    return pImgDst;
}



UIOBJECT_CLASS_NAME_IMP(CMediaAlbumArtCtrl, "AlbumArt")

CMediaAlbumArtCtrl::CMediaAlbumArtCtrl() {
    m_msgNeed = UO_MSG_WANT_LBUTTON;

    m_clrBg.set(RGB(0, 0, 0));
    m_nCurAlbumArt = 0;
    m_rcFrameMaskPos.setEmpty();
}

CMediaAlbumArtCtrl::~CMediaAlbumArtCtrl() {
    m_pSkin->unregisterTimerObject(this);
}

void CMediaAlbumArtCtrl::onCreate() {
    CUIObject::onCreate();

    registerHandler(CMPlayerAppBase::getEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

    m_pSkin->registerTimerObject(this, 20 * 1000);

    updateAlbumArt();
}

void CMediaAlbumArtCtrl::onEvent(const IEvent *pEvent) {
    if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED) {
        m_pSkin->unregisterTimerObject(this);
        m_pSkin->registerTimerObject(this, 20 * 1000);
        updateAlbumArt();
        invalidate();
    }
}

bool CMediaAlbumArtCtrl::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (m_id != UID_INVALID) {
        m_pSkin->postCustomCommandMsg(m_id);

        return true;
    }
    return false;
}

bool CMediaAlbumArtCtrl::onLButtonDown(uint32_t nFlags, CPoint point) {
    return false;
}

void CMediaAlbumArtCtrl::onTimer(int nId) {
    if (m_mediaAlbumArt.getPicCount() > 1) {
        m_nCurAlbumArt++;
        if (m_nCurAlbumArt >= m_mediaAlbumArt.getPicCount()) {
            m_nCurAlbumArt = 0;
        }

        RawImageData *prawImg = m_mediaAlbumArt.loadAlbumArtByIndex(m_nCurAlbumArt);
        if (prawImg) {
            createAlbumArtImage(prawImg);
            return;
        }
        invalidate();
    }
}

void CMediaAlbumArtCtrl::onSize() {
    CUIObject::onSize();

    if (!isMaskImageUsed()) {
        resizeAlbumArt();
    }
}

void CMediaAlbumArtCtrl::draw(CRawGraph *canvas) {
    CUIObject::draw(canvas);

    if (!m_img.isValid()) {
        return;
    }

    if (isMaskImageUsed()) {
        m_img.blt(canvas, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), 0, 0, BPM_BLEND);
        return;
    }

    //
    // No album art mask image is used.
    //
    CRect rc = m_rcObj;

    if (m_img.m_cx == 0 || m_img.m_cy == 0 || rc.empty()) {
        return;
    }

    int w, h;
    w = m_img.m_cx * rc.height() / m_img.m_cy;
    if (w < rc.width()) {
        // fill left and right
        h = rc.height();
    } else {
        // fill top and bottom
        h = m_img.m_cy * rc.width() / m_img.m_cx;
    }

    m_img.blt(canvas, rc.left + (rc.width() - w) / 2, rc.top + (rc.height() - h) / 2, BPM_BLEND);
}

bool CMediaAlbumArtCtrl::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        m_strImgFile = szValue;
        m_img.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (strcasecmp(szProperty, "FrameMask") == 0) {
        m_strFrmMask = szValue;
        m_frmMask.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (strcasecmp(szProperty, "FrameMaskRectPos") == 0) {
        getRectValue(szValue, m_rcFrameMaskPos);
    } else if (strcasecmp(szProperty, "BgColor") == 0) {
        getColorValue(m_clrBg, szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CMediaAlbumArtCtrl::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropImageFile(SZ_PN_IMAGE, m_strImgFile.c_str(), true);
}
#endif // _SKIN_EDITOR_

void CMediaAlbumArtCtrl::updateAlbumArt() {
    m_mediaAlbumArt.close();
    m_nCurAlbumArt = 0;
    m_img.destroy();
    m_imgOrg.destroy();

    if (m_mediaAlbumArt.load() == ERR_OK) {
        RawImageData *prawImg = m_mediaAlbumArt.loadAlbumArtByIndex(m_nCurAlbumArt);
        if (prawImg) {
            createAlbumArtImage(prawImg);
            return;
        }
    }

    if (isMaskImageUsed()) {
        m_img.loadFromSRM(m_pSkin->getSkinFactory(), m_strImgFile.c_str());
    } else {
        m_imgOrg.loadFromSRM(m_pSkin->getSkinFactory(), m_strImgFile.c_str());
        resizeAlbumArt();
    }
}

void clearRawImage(RawImageData *pImg, const CColor &clr);

void CMediaAlbumArtCtrl::createAlbumArtImage(RawImageData *pImgSrc) {
    if (!isMaskImageUsed()) {
        m_imgOrg.destroy();
        m_imgOrg.attach(pImgSrc);

        resizeAlbumArt();
        return;
    }

    assert(pImgSrc);
    if (!pImgSrc) {
        return;
    }

    //
    // Album art mask image is used.
    //
    RawImageData *pImg, *pImgSizedAlbumArt;
    int w, h;
    CRawImage imgSizedAlbumArt;
    CSFImage imgAaFrame;

    if (pImgSrc) {
        pImgSrc = convertTo32BppRawImage(pImgSrc);
    }
    assert(pImgSrc);
    if (!pImgSrc) {
        return;
    }

    // create a scale sized image for album art
    pImgSizedAlbumArt = createScaleImage(pImgSrc, m_rcFrameMaskPos.width(), m_rcFrameMaskPos.height());
    imgSizedAlbumArt.attach(pImgSizedAlbumArt);

    // load album art frame
    imgAaFrame.loadFromSRM(m_pSkin->getSkinFactory(), m_strImgFile.c_str());
    if (!imgAaFrame.isValid()) {
        freeRawImage(pImgSrc);
        return;
    }
    pImg = new RawImageData;
    if (!pImg->create(imgAaFrame.width(), imgAaFrame.height(), imgAaFrame.getHandle()->bitCount)) {
        delete pImg;
        freeRawImage(pImgSrc);
        return;
    }

    CColor clrBg;

    clrBg.set(0);
    clrBg.setAlpha(0);
    rawImageSet(pImg, clrBg.r(), clrBg.g(), clrBg.b(), clrBg.getAlpha());
    m_img.attach(pImg);

    //
    // Album Art --> add mask --> blt to Frame
    //
    w = m_rcFrameMaskPos.width();
    h = m_rcFrameMaskPos.height();
    if (w == pImgSizedAlbumArt->width) {
        int hTop = pImgSizedAlbumArt->height / 2;
        int hBottom = pImgSizedAlbumArt->height - hTop;

        // blt top mask
        m_frmMask.blt(&imgSizedAlbumArt, 0, 0, w, hTop, 0, 0, BPM_MULTIPLY);

        // blt top to frame
        imgSizedAlbumArt.blt(&imgAaFrame, m_rcFrameMaskPos.left, m_rcFrameMaskPos.top, w, hTop, 0, 0, BPM_BLEND);

        // blt top to Image
        imgAaFrame.blt(&m_img, 0, m_img.height() / 2 - (hTop + m_rcFrameMaskPos.top), m_img.width(), hTop + m_rcFrameMaskPos.top, 0, 0, BPM_COPY);


        // blt bottom mask
        m_frmMask.blt(&imgSizedAlbumArt, 0, hTop, w, hBottom, 0, m_frmMask.height() - hBottom, BPM_MULTIPLY);

        // blt bottom to frame
        imgSizedAlbumArt.blt(&imgAaFrame, m_rcFrameMaskPos.left, m_rcFrameMaskPos.bottom - hBottom, w, hBottom, 0, hTop, BPM_BLEND);

        // blt bottom to Image
        imgAaFrame.blt(&m_img, 0, m_img.height() / 2, m_img.width(), hBottom + (m_img.height() - m_rcFrameMaskPos.bottom), 0, m_rcFrameMaskPos.bottom - hBottom, BPM_COPY);
    } else {
        int wLeft = pImgSizedAlbumArt->width / 2;
        int wRight = pImgSizedAlbumArt->width - wLeft;

        // blt left mask
        m_frmMask.blt(&imgSizedAlbumArt, 0, 0, wLeft, h, 0, 0, BPM_MULTIPLY);

        // blt left to frame
        imgSizedAlbumArt.blt(&imgAaFrame, m_rcFrameMaskPos.left, m_rcFrameMaskPos.top, wLeft, h, 0, 0, BPM_BLEND);

        // blt left to Image
        imgAaFrame.blt(&m_img, m_img.width() / 2 - (wLeft + m_rcFrameMaskPos.left), 0, wLeft + m_rcFrameMaskPos.left, m_img.height(), 0, 0, BPM_COPY);


        // blt right mask
        m_frmMask.blt(&imgSizedAlbumArt, wLeft, 0, wRight, h, m_frmMask.width() - wRight, 0, BPM_MULTIPLY);

        // blt right to frame
        imgSizedAlbumArt.blt(&imgAaFrame, m_rcFrameMaskPos.right - wRight, m_rcFrameMaskPos.top, wRight, h, wLeft, 0, BPM_BLEND);

        // blt right to Image
        imgAaFrame.blt(&m_img, m_img.width() / 2, 0, wRight + (m_img.width() - m_rcFrameMaskPos.right), m_img.height(), m_rcFrameMaskPos.right - wRight, 0, BPM_COPY);
    }

    freeRawImage(pImgSrc);
}

void CMediaAlbumArtCtrl::resizeAlbumArt() {
    if (isMaskImageUsed()) {
        return;
    }
    if (!m_imgOrg.isValid()) {
        return;
    }

    m_img.destroy();

    RawImageData *pImgSizedAlbumArt;

    pImgSizedAlbumArt = createScaleImage(m_imgOrg.getHandle(), m_rcObj.width(), m_rcObj.height());
    if (pImgSizedAlbumArt) {
        m_img.attach(pImgSizedAlbumArt);
    }
}
