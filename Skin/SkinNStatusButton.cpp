// SkinNStatusButton.cpp: implementation of the CSkinNStatusButton class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinNStatusButton.h"

#define FADE_BUTTON_TIME_OUT        300

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinNStatusButton, "NStatusButton")

CSkinNStatusButton::CSkinNStatusButton()
{
    m_msgNeed = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_ENTER_KEY;

    m_xExtendStart = m_xExtendEnd = -1;
    m_bTile = true;

    m_bLBtDown = false;
    m_nCurStatus = 0;

    m_bHover = false;
    m_bEnableHover = false;

    m_bContinuousBegin = false;
    m_bContinuousCmd = false;
    m_bpm = BPM_BLEND;
    m_nTimerIdContinuous = 0;

    m_bFadein = true;
    m_nTimerIdFadein = 0;
    m_dwBeginFadeinTime = 0;
    m_pLastImage = nullptr;
}

CSkinNStatusButton::~CSkinNStatusButton()
{
    destroy();
}

bool CSkinNStatusButton::onLButtonDown(uint32_t nFlags, CPoint point)
{
    assert(m_pSkin);

    if (!isMouseHitBt(point))
        return false;

    if (m_bHover)
        m_bHover = false;
    else
        m_pSkin->setCaptureMouse(this);

    buttonDownAction();

    return true;
}

void CSkinNStatusButton::onTimer(int nId)
{
    if (m_nTimerIdFadein != 0)
    {
        // In fade in, just redraw
        invalidate();
        return;
    }

    if (m_bLBtDown)
    {
        CPoint pt = getCursorPos();
        m_pSkin->screenToClient(pt);

        if (isMouseHitBt(pt))
        {
            if (m_id != UID_INVALID)
                m_pSkin->postCustomCommandMsg(m_id);

            if (m_nCurStatus >= 0 && m_nCurStatus < (int)m_vBtStatImg.size())
            {
                if (m_vBtStatImg[m_nCurStatus]->nIDCmd != UID_INVALID)
                    m_pSkin->postCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);
            }
        }

        if (m_bContinuousBegin)
        {
            if (m_nTimerIdContinuous)
                m_pSkin->unregisterTimerObject(this, m_nTimerIdContinuous);
            m_nTimerIdContinuous = m_pSkin->registerTimerObject(this, 100);
            m_bContinuousBegin = false;
        }
    }
    else
    {
        m_pSkin->unregisterTimerObject(this, m_nTimerIdContinuous);
        m_nTimerIdContinuous = 0;
    }
}

bool CSkinNStatusButton::onLButtonUp(uint32_t nFlags, CPoint point)
{
    m_pSkin->releaseCaptureMouse(this);

    if (!m_bLBtDown)
        return true;

    buttonUpAction();

    return true;
}

bool CSkinNStatusButton::onMouseDrag(CPoint point)
{
    bool    bStatusChanged = false;

    if (!isMouseHitBt(point))
    {
        m_bLBtDown = false;
        bStatusChanged = true;
    }
    else if (!m_bLBtDown)
    {
        m_bLBtDown = true;
        bStatusChanged = true;

        if (m_bHover)
            m_bHover = false;
        else
            m_pSkin->setCaptureMouse(this);
    }

    if (bStatusChanged)
    {
        if (m_nTimerIdFadein == 0)
            startFadeDrawTimer(m_bLBtDown, m_bHover);
        invalidate();
    }

    return true;
}

bool CSkinNStatusButton::onMouseMove(CPoint point)
{
    bool    bStatusChanged = false;

    if (isMouseHitBt(point))
    {
        if (!m_bLBtDown && m_bEnableHover && !m_bHover)
        {
            m_bHover = true;
            m_pSkin->setCaptureMouse(this);

            bStatusChanged = true;
        }
    }
    else
    {
        if (m_bHover)
        {
            m_bHover = false;

            m_pSkin->releaseCaptureMouse(this);

            bStatusChanged = true;
        }
    }

    if (bStatusChanged)
    {
        if (m_nTimerIdFadein == 0)
            startFadeDrawTimer(m_bLBtDown, m_bHover);
        invalidate();
    }

    return true;
}

void CSkinNStatusButton::onKeyUp(uint32_t nChar, uint32_t nFlags)
{
    if (nChar == VK_SPACE)
        buttonUpAction();
}

void CSkinNStatusButton::onKeyDown(uint32_t nChar, uint32_t nFlags)
{
    if (nChar == VK_SPACE && !m_bLBtDown)
        buttonDownAction();
    else if (nChar == VK_RETURN)
    {
        buttonDownAction();
        buttonUpAction();
    }
}

void CSkinNStatusButton::onSetFocus()
{
    m_bHover = true;
    invalidate();
}

void CSkinNStatusButton::onKillFocus()
{
    if (m_bLBtDown)
        m_bLBtDown = false;

    m_bHover = false;
    invalidate();
}

void CSkinNStatusButton::draw(CRawGraph *canvas)
{
    if (m_vBtStatImg.empty())
        return;

    if (m_nCurStatus < 0 || m_nCurStatus >= (int)m_vBtStatImg.size())
        m_nCurStatus = 0;

    BtStatImg        *btimg = m_vBtStatImg[m_nCurStatus];
    CSFImage        *pImage = nullptr;

    if (m_enable)
    {
        if (m_bLBtDown && btimg->imgSel.isValid())
            pImage = &btimg->imgSel;
        else if (m_bEnableHover && m_bHover && btimg->imgHover.isValid())
            pImage = &btimg->imgHover;
        else if (btimg->imgBk.isValid())
            pImage = &btimg->imgBk;
    }
    else
    {
        if (btimg->imgDisabled.isValid())
            pImage = &btimg->imgDisabled;
        else if (btimg->imgBk.isValid())
            pImage = &btimg->imgBk;
    }

    if (!pImage)
        return;


    if (m_nTimerIdFadein != 0)
    {
        uint32_t        dwTimeCur = getTickCount();

        if (dwTimeCur - m_dwBeginFadeinTime >= FADE_BUTTON_TIME_OUT)
        {
            m_pSkin->unregisterTimerObject(this, m_nTimerIdFadein);
            m_nTimerIdFadein = 0;
        }
        else
        {
            int            nAlpha = (dwTimeCur - m_dwBeginFadeinTime) * 255 / FADE_BUTTON_TIME_OUT;
            fadeInDrawBt(canvas, pImage, nAlpha);
            return;
        }
    }

    if (m_xExtendStart == -1)
    {
        if (m_imgMask.isValid())
            pImage->maskBlt(canvas, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
        else
            pImage->blt(canvas, m_rcObj.left, m_rcObj.top, m_bpm);
    }
    else
    {
        pImage->xScaleBlt(canvas, m_rcObj.left, m_rcObj.top,
            m_rcObj.width(), m_rcObj.height(),
            m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
    }
}

void CSkinNStatusButton::fadeInDrawBt(CRawGraph *canvas, CSFImage *pImage, int nAlpha)
{
    assert(m_pLastImage && m_bFadein);

    int        nAlphaOld = canvas->getOpacityPainting();

    canvas->setOpacityPainting((255 - nAlpha) * nAlphaOld / 255);

    if (m_xExtendStart == -1)
    {
        if (m_imgMask.isValid())
            m_pLastImage->maskBlt(canvas, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
        else
            m_pLastImage->blt(canvas, m_rcObj.left, m_rcObj.top, m_bpm);
    }
    else
    {
        m_pLastImage->xScaleBlt(canvas, m_rcObj.left, m_rcObj.top,
            m_rcObj.width(), m_rcObj.height(),
            m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
    }

    canvas->setOpacityPainting(nAlpha * nAlphaOld / 255);

    if (m_xExtendStart == -1)
    {
        if (m_imgMask.isValid())
            pImage->maskBlt(canvas, m_rcObj.left, m_rcObj.top, &m_imgMask, m_bpm);
        else
            pImage->blt(canvas, m_rcObj.left, m_rcObj.top, m_bpm);
    }
    else
    {
        pImage->xScaleBlt(canvas, m_rcObj.left, m_rcObj.top,
            m_rcObj.width(), m_rcObj.height(),
            m_xExtendStart, m_xExtendEnd, m_bTile, m_bpm);
    }

    canvas->setOpacityPainting(nAlphaOld);
}


bool CSkinNStatusButton::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CUIObject::setProperty(szProperty, szValue))
        return true;

    if (strcasecmp(szProperty, "CurStatus") == 0)
        m_nCurStatus = atoi(szValue);
    else if (szProperty[0] == 'S' && isDigit(szProperty[1]) && szProperty[2] == '_')
    {
        cstr_t        szPropertyNew = szProperty + 3;
        int        n = szProperty[1] - '0';
        if (n >= (int)m_vBtStatImg.size())
            setMaxStat(n + 1);
        BtStatImg    *btimg = m_vBtStatImg[n];

        if (strcasecmp(szPropertyNew, SZ_PN_IMAGE) == 0)
        {
            btimg->strBkFile = szValue;
            btimg->imgBk.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
            if (!btimg->imgBk.isValid())
                DBG_LOG1("load image file: %s, FAILED", szValue);
            assert(btimg->imgBk.isValid());
        }
        else if (isPropertyName(szPropertyNew, SZ_PN_IMAGE_SIZE))
        {
            scan2IntX(szValue, btimg->imgBk.m_cx, btimg->imgBk.m_cy);
        }
        else if (isPropertyName(szPropertyNew, SZ_PN_IMAGE_POS))
        {
            scan2IntX(szValue, btimg->imgBk.m_x, btimg->imgBk.m_y);
        }
        else if (isPropertyName(szPropertyNew, "ImageSelPos"))
        {
            btimg->imgSel.loadFromSRM(m_pSkin->getSkinFactory(), btimg->strBkFile.c_str());
            btimg->imgSel.m_cx = btimg->imgBk.m_cx;
            btimg->imgSel.m_cy = btimg->imgBk.m_cy;
            scan2IntX(szValue, btimg->imgSel.m_x, btimg->imgSel.m_y);
        }
        else if (isPropertyName(szPropertyNew, "ImageFocusPos"))
        {
            btimg->imgHover.loadFromSRM(m_pSkin->getSkinFactory(), btimg->strBkFile.c_str());
            btimg->imgHover.m_cx = btimg->imgBk.m_cx;
            btimg->imgHover.m_cy = btimg->imgBk.m_cy;
            scan2IntX(szValue, btimg->imgHover.m_x, btimg->imgHover.m_y);
            m_bEnableHover = true;
        }
        else if (isPropertyName(szPropertyNew, "ImageDisabledPos"))
        {
            btimg->imgDisabled.loadFromSRM(m_pSkin->getSkinFactory(), btimg->strBkFile.c_str());
            btimg->imgDisabled.m_cx = btimg->imgBk.m_cx;
            btimg->imgDisabled.m_cy = btimg->imgBk.m_cy;
            scan2IntX(szValue, btimg->imgDisabled.m_x, btimg->imgDisabled.m_y);
        }
        else if (isPropertyName(szPropertyNew, "IDCmd"))
        {
            btimg->nIDCmd = m_pSkin->getSkinFactory()->getIDByName(szValue);
        }

        // Old property styles, decrepted
        else if (strcasecmp(szPropertyNew, SZ_PN_IMAGERECT) == 0)
        {
            if (!getRectValue(szValue, btimg->imgBk))
                goto PARSE_VALUE_FAILED;
        }
        else if (strcasecmp(szPropertyNew, "ImageSel") == 0)
        {
            btimg->strSelFile = szValue;
            btimg->imgSel.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
        }
        else if (strcasecmp(szPropertyNew, "ImageSelRect") == 0)
        {
            if (!getRectValue(szValue, btimg->imgSel))
                goto PARSE_VALUE_FAILED;
        }
        else if (strcasecmp(szPropertyNew, "ImageFocus") == 0)
        {
            btimg->strHoverFile = szValue;
            btimg->imgHover.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
            if (n == m_nCurStatus)
                m_bEnableHover = true;
        }
        else if (strcasecmp(szPropertyNew, "ImageFocusRect") == 0)
        {
            if (!getRectValue(szValue, btimg->imgHover))
                goto PARSE_VALUE_FAILED;
        }
        else if (strcasecmp(szPropertyNew, "ImageDisabled") == 0)
        {
            btimg->strDisabledFile = szValue;
            btimg->imgDisabled.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
        }
        else if (strcasecmp(szPropertyNew, "ImageDisabledRect") == 0)
        {
            if (!getRectValue(szValue, btimg->imgDisabled))
                goto PARSE_VALUE_FAILED;
        }
        else if (strcasecmp(szProperty, "IDCmd") == 0)
        {
            if (m_vBtStatImg.size() == 0)
                m_vBtStatImg.push_back(new BtStatImg);
            m_vBtStatImg[0]->nIDCmd = m_pSkin->getSkinFactory()->getIDByName(szValue);
        }
        else
            return false;
    }
    else if (strcasecmp(szProperty, "Continuous") == 0)
    {
        m_bContinuousCmd = isTRUE(szValue);
    }
    else if (isPropertyName(szProperty, "ExtendPos"))
    {
        scan2IntX(szValue, m_xExtendStart, m_xExtendEnd);
    }
    else if (isPropertyName(szProperty, "Tile"))
    {
        m_bTile = isTRUE(szValue);
    }
    else if (isPropertyName(szProperty, "BlendPixMode"))
    {
        m_bpm = blendPixModeFromStr(szValue);
    }
    else if (isPropertyName(szProperty, "Fadein"))
    {
        m_bFadein = isTRUE(szValue);
    }
    else
        return false;

    return true;

PARSE_VALUE_FAILED:
    ERR_LOG2("Analyse Value: %s = %s FAILED.", szProperty, szValue);
    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinNStatusButton::enumProperties(CUIObjProperties &listProperties)
{
    CUIObject::enumProperties(listProperties);

    listProperties.addPropInt("CurStatus", m_nCurStatus);

    for (int i = 0; i < (int)m_vBtStatImg.size(); i++)
    {
        BtStatImg    *btimg = m_vBtStatImg[i];
        string        name, strNameRect;
        string        strPrefix = "S0_";
        strPrefix[1] = '0' + i;

        name = strPrefix + "Image";
        strNameRect = strPrefix + "ImageRect";
        listProperties.addPropImage(name.c_str(), strNameRect.c_str(), btimg->strBkFile.c_str(), btimg->imgBk);

        name = strPrefix + "ImageSel";
        strNameRect = strPrefix + "ImageSelRect";
        listProperties.addPropImage(name.c_str(), strNameRect.c_str(), btimg->strSelFile.c_str(), btimg->imgSel, !btimg->strSelFile.empty());

        name = strPrefix + "ImageFocus";
        strNameRect = strPrefix + "ImageFocusRect";
        listProperties.addPropImage(name.c_str(), strNameRect.c_str(), btimg->strHoverFile.c_str(), btimg->imgHover, !btimg->strHoverFile.empty());

        name = strPrefix + "ImageDisabled";
        strNameRect = strPrefix + "ImageDisabledRect";
        listProperties.addPropImage(name.c_str(), strNameRect.c_str(), btimg->strDisabledFile.c_str(), btimg->imgDisabled, !btimg->strDisabledFile.empty());

        listProperties.addPropID("IDCmd", m_pSkin->getSkinFactory()->getStringOfID(btimg->nIDCmd).c_str(), btimg->nIDCmd != UID_INVALID);
    }

    listProperties.addPropBoolStr("Continuous", m_bContinuousCmd, m_bContinuousCmd);

    listProperties.addPropBlendPixMode("BlendPixMode", m_bpm, m_bpm != BPM_BLEND);
}
#endif // _SKIN_EDITOR_

int CSkinNStatusButton::getStatus()
{
    return m_nCurStatus;
}

void CSkinNStatusButton::setStatus(int nStatus)
{
    if (nStatus == m_nCurStatus)
        return;
    if (nStatus >= 0 && nStatus < (int)m_vBtStatImg.size())
    {
        m_nCurStatus = nStatus;
        invalidate();
    }
}

void CSkinNStatusButton::setMaxStat(int nMaxStat)
{
    if ((int)m_vBtStatImg.size() > nMaxStat)
    {
        // remove extra stat
        for (int i = m_vBtStatImg.size() - 1; i >= nMaxStat; i--)
        {
            delete m_vBtStatImg.back();
            m_vBtStatImg.pop_back();
        }
    }
    else if ((int)m_vBtStatImg.size() < nMaxStat)
    {
        // add extra stat
        for (int i = m_vBtStatImg.size(); i < nMaxStat; i++)
        {
            m_vBtStatImg.push_back(new BtStatImg);
        }
    }
}

void CSkinNStatusButton::buttonDownAction()
{
    if (m_bFadein)
    {
        startFadeDrawTimer(false, true);
    }

    if (m_bContinuousCmd)
    {
        m_bContinuousBegin = true;
        m_nTimerIdContinuous = m_pSkin->registerTimerObject(this, 500);
    }

    m_bLBtDown = true;

    invalidate();
}

void CSkinNStatusButton::buttonUpAction()
{
    if (m_bContinuousCmd)
    {
        m_pSkin->unregisterTimerObject(this, m_nTimerIdContinuous);
        m_nTimerIdContinuous = 0;
    }

    if (m_bFadein)
        startFadeDrawTimer(true, false);

    m_bLBtDown = false;

    if (m_nCurStatus >= 0 && m_nCurStatus < (int)m_vBtStatImg.size() - 1)
        m_nCurStatus++;
    else
        m_nCurStatus = 0;

    if (m_nCurStatus < (int)m_vBtStatImg.size())
        m_bEnableHover = m_vBtStatImg[m_nCurStatus]->imgHover.isValid();

    invalidate();

    if (m_id != UID_INVALID)
        m_pSkin->postCustomCommandMsg(m_id);

    if (m_vBtStatImg[m_nCurStatus]->nIDCmd != UID_INVALID)
        m_pSkin->postCustomCommandMsg(m_vBtStatImg[m_nCurStatus]->nIDCmd);
}

void CSkinNStatusButton::startFadeDrawTimer(bool bPrevButtonDown, bool bPrevHover)
{
    assert(m_nCurStatus >= 0 && m_nCurStatus <= (int)m_vBtStatImg.size());
    if (m_nTimerIdFadein != 0)
        m_pSkin->unregisterTimerObject(this, m_nTimerIdFadein);
    m_nTimerIdFadein = m_pSkin->registerTimerObject(this, 50);
    m_dwBeginFadeinTime = getTickCount();

    BtStatImg    *btimg = m_vBtStatImg[m_nCurStatus];

    m_pLastImage = nullptr;
    if (bPrevButtonDown && btimg->imgSel.isValid())
        m_pLastImage = &btimg->imgSel;
    else if (m_bEnableHover && bPrevHover && btimg->imgHover.isValid())
        m_pLastImage = &btimg->imgHover;
    else if (btimg->imgBk.isValid())
        m_pLastImage = &btimg->imgBk;
}

void CSkinNStatusButton::destroy()
{
    for (int i = 0; i < (int)m_vBtStatImg.size(); i++)
    {
        delete m_vBtStatImg[i];
    }
    m_vBtStatImg.clear();
}

bool CSkinNStatusButton::isMouseHitBt(CPoint &pt)
{
    if (m_rcPadding.left == 0 && m_rcPadding.top == 0
        && m_rcPadding.right == 0 && m_rcPadding.bottom == 0)
        return m_rcObj.ptInRect(pt);

    CRect    rc = m_rcObj;

    rc.left -= m_rcPadding.left;
    rc.top -= m_rcPadding.top;
    rc.right += m_rcPadding.right;
    rc.bottom += m_rcPadding.bottom;
    return rc.ptInRect(pt);
}
