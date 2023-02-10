#include "SkinTypes.h"
#include "Skin.h"
#include "SkinVScrollBar.h"


//////////////////////////////////////////////////////////////////////////

CSkinSrollBarBase::CSkinSrollBarBase() {
    // 虚拟滚动位置
    m_nVirtualMin = 0; // 最小的值
    m_nVirtualMax = 10; // 最大的值
    m_nVirtualPage = 1; // 滚动一页的值
    m_nVirtualLine = 1; // 滚动一行的值，缺省为1
    m_nVirtualCurPos = m_nVirtualMin;

    m_nPosThumb = 0; // 滚动条的位置(上面按钮的下方 －> thumb的上方的距离)

    m_bStretchedThumb = false;
    m_nSizeThumbStretched = 41;

    m_msgNeed = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_MOUSEWHEEL;

    m_nClickDelayTimerId = 0;
    m_nRepeatTimerId = 0;

    m_pScrollNotify = nullptr;

    m_nCursorToThumbBeg = 0;
    m_PushDownPos = PUSH_DOWN_NONE;
    m_CursorPosLatest = PUSH_DOWN_NONE;

    m_nLinesPerWheel = 1;
}

CSkinSrollBarBase::~CSkinSrollBarBase() {
    unregisterTimer();
}

void CSkinSrollBarBase::setScrollInfo(int nMin, int nMax, int nPage, int nPos, int nLine, bool bRedraw) {
    m_nVirtualMin = nMin;
    m_nVirtualMax = nMax - nPage;
    m_nVirtualPage = nPage;
    m_nVirtualLine = nLine;

    m_nVirtualCurPos = nPos;

    if (m_nVirtualMax < 0) {
        m_nVirtualMax = 0;
    }

    xcreaseVirtualPos(0);

    adjustThumbSize();

    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

    if (bRedraw) {
        invalidate();
    }
}

// COMMENT:
//        设置新的滚动位置
// RETURN:
//        返回设置以前的滚动位置的值
int CSkinSrollBarBase::setScrollPos(int nPos, bool bRedraw) {
    int nPosOld;

    nPosOld = m_nVirtualCurPos;
    m_nVirtualCurPos = nPos;

    xcreaseVirtualPos(0);

    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

    if (bRedraw) {
        invalidate();
    }

    return nPosOld;
}

int CSkinSrollBarBase::getMax() const {
    return m_nVirtualMax;
}

void CSkinSrollBarBase::setScrollNotify(IScrollNotify *pNofity) {
    m_pScrollNotify = pNofity;
}

bool CSkinSrollBarBase::isEnabled() const {
    return m_nVirtualMax != 0;
}

void CSkinSrollBarBase::disableScrollBar() {
    m_nVirtualMin = 0;
    m_nVirtualMax = 0;
    m_nVirtualPage = 1;
    m_nVirtualLine = 1;

    m_nPosThumb = 0;

    if (m_nVirtualCurPos != 0) {
        m_nVirtualCurPos = 0;
        onScroll(SB_THUMBPOSITION);
    }
}

void CSkinSrollBarBase::onSize() {
    CUIObject::onSize();

    adjustThumbSize();

    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);
}

bool CSkinSrollBarBase::onLButtonDown(uint32_t nFlags, CPoint point) {
    assert(m_pSkin);

    m_PushDownPos = getPushDownPos(point);
    m_CursorPosLatest = m_PushDownPos;

    switch (m_PushDownPos) {
    case PUSH_DOWN_TOPBT:
        // 滚动条上面的按钮
        topBtOnLButtonDown(nFlags, point);
        break;
    case PUSH_DOWN_BOTTOMBT:
        // 滚动条下面的按钮
        bottomBtOnLButtonDown(nFlags, point);
        break;
    case PUSH_DOWN_THUMB:
        // 滚动条的拖动box(thumb)
        thumbOnLButtonDown(nFlags, point);
        break;
    case PUSH_DOWN_TOPTRACK:
    case PUSH_DOWN_BOTTOMTRACK:
        // 滚动条的空白位置，进行翻页操作
        trackOnLButtonDown(nFlags, point);
        break;
    default:
        break;
    }

    return true;
}

void CSkinSrollBarBase::topBtOnLButtonDown(uint32_t nFlags, CPoint point) {
    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

    int nVirtualPosOld = m_nVirtualCurPos;
    xcreaseVirtualPos(-m_nVirtualLine);
    if (m_nVirtualCurPos == nVirtualPosOld) {
        invalidate();
        return;
    }

    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

    onScroll(SB_LINEUP);

    registerTimer();
    // DBG_LOG0("Top button down");

    invalidate();
}

void CSkinSrollBarBase::bottomBtOnLButtonDown(uint32_t nFlags, CPoint point) {
    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

    int nVirtualPosOld = m_nVirtualCurPos;
    xcreaseVirtualPos(m_nVirtualLine);
    if (m_nVirtualCurPos == nVirtualPosOld) {
        invalidate();
        return;
    }

    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);
    onScroll(SB_LINEDOWN);

    registerTimer();
    // DBG_LOG0("Bottom button down");

    invalidate();
}

void CSkinSrollBarBase::trackOnLButtonDown(uint32_t nFlags, CPoint point) {
    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

    registerTimer();
    // DBG_LOG0("Track button down");

    if (m_PushDownPos == PUSH_DOWN_TOPTRACK) {
        int nVirtualPosOld = m_nVirtualCurPos;
        xcreaseVirtualPos(-m_nVirtualPage);
        if (m_nVirtualCurPos == nVirtualPosOld) {
            return;
        }
        onScroll(SB_PAGEUP);
    } else if (m_PushDownPos == PUSH_DOWN_BOTTOMTRACK) {
        int nVirtualPosOld = m_nVirtualCurPos;
        xcreaseVirtualPos(m_nVirtualPage);
        if (m_nVirtualCurPos == nVirtualPosOld) {
            return;
        }
        onScroll(SB_PAGEDOWN);
    }

    invalidate();
}

bool CSkinSrollBarBase::onLButtonUp(uint32_t nFlags, CPoint point) {
    // DBG_LOG0("onLButtonUp ");
    switch (m_PushDownPos) {
    case PUSH_DOWN_TOPBT:
        // 滚动条上面的按钮
        topBtOnLButtonUp(nFlags, point);
        break;
    case PUSH_DOWN_BOTTOMBT:
        // 滚动条下面的按钮
        bottomBtOnLButtonUp(nFlags, point);
        break;
    case PUSH_DOWN_THUMB:
        // 滚动条的拖动box(thumb)
        thumbOnLButtonUp(nFlags, point);
        break;
    case PUSH_DOWN_TOPTRACK:
    case PUSH_DOWN_BOTTOMTRACK:
        // 滚动条的空白位置，进行翻页操作
        trackOnLButtonUp(nFlags, point);
        break;
    default:
        break;
    }

    m_PushDownPos = PUSH_DOWN_NONE;

    //    PUSH_DOWN_POS        CursorPosNew;
    //    CursorPosNew = getPushDownPos(point);

    invalidate();

    return true;
}

void CSkinSrollBarBase::topBtOnLButtonUp(uint32_t nFlags, CPoint point) {
    unregisterTimer();

    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    // DBG_LOG0("TopBt button up");
}

void CSkinSrollBarBase::bottomBtOnLButtonUp(uint32_t nFlags, CPoint point) {
    unregisterTimer();

    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    // DBG_LOG0("BottomBt button up");
}

void CSkinSrollBarBase::thumbOnLButtonUp(uint32_t nFlags, CPoint point) {
    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    // DBG_LOG0("Thumb button up");
}

void CSkinSrollBarBase::trackOnLButtonUp(uint32_t nFlags, CPoint point) {
    onTimer(m_nRepeatTimerId);

    unregisterTimer();

    // 释放鼠标输入
    m_pSkin->releaseCaptureMouse(this);

    // DBG_LOG0("Track button up");
}

void CSkinSrollBarBase::onMouseWheel(int nWheelDistance, int nMkeys, CPoint pt) {
    if (nMkeys == 0) {
        int nVirtualPosOld = m_nVirtualCurPos;
        xcreaseVirtualPos(m_nVirtualLine * nWheelDistance * m_nLinesPerWheel);
        if (m_nVirtualCurPos == nVirtualPosOld) {
            invalidate();
            return;
        }

        m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

        onScroll(SB_THUMBPOSITION);

        invalidate();
    }
}

bool CSkinSrollBarBase::onMouseDrag(CPoint point) {
    switch (m_PushDownPos) {
    case PUSH_DOWN_THUMB:
        // 滚动条的拖动box(thumb)
        thumbOnMouseDrag(point);
        break;
    default:
        break;
    }

    return true;
}

bool CSkinSrollBarBase::onMouseMove(CPoint point) {
    switch (m_PushDownPos) {
    case PUSH_DOWN_TOPBT:
        // 滚动条上面的按钮
        topBtOnMouseMove(point);
        break;
    case PUSH_DOWN_BOTTOMBT:
        // 滚动条下面的按钮
        bottomBtOnMouseMove(point);
        break;
    case PUSH_DOWN_TOPTRACK:
    case PUSH_DOWN_BOTTOMTRACK:
        // 滚动条的空白位置，进行翻页操作
        trackOnMouseDrag(point);
        break;
    case PUSH_DOWN_NONE:
        {
            PUSH_DOWN_POS posNew = getPushDownPos(point);


            if (posNew != m_CursorPosLatest) {
                m_CursorPosLatest = posNew;
                invalidate();
            }
        }
        break;
    default:
        break;
    }

    return true;
}

void CSkinSrollBarBase::topBtOnMouseMove(CPoint point) {
    PUSH_DOWN_POS CursorPosNew;

    CursorPosNew = getPushDownPos(point);

    if (CursorPosNew != m_CursorPosLatest) {
        m_CursorPosLatest = CursorPosNew;
        invalidate();
    }
}

void CSkinSrollBarBase::bottomBtOnMouseMove(CPoint point) {
    PUSH_DOWN_POS CursorPosNew;

    CursorPosNew = getPushDownPos(point);

    if (CursorPosNew != m_CursorPosLatest) {
        m_CursorPosLatest = CursorPosNew;
        invalidate();
    }
}

void CSkinSrollBarBase::trackOnMouseDrag(CPoint point) {
    PUSH_DOWN_POS CursorPosNew;

    CursorPosNew = getPushDownPos(point);

    if (CursorPosNew != m_CursorPosLatest) {
        m_CursorPosLatest = CursorPosNew;
        invalidate();
    }
}

void CSkinSrollBarBase::onTimer(int nId) {
    if (nId == m_nClickDelayTimerId) {
        m_pSkin->unregisterTimerObject(this, m_nClickDelayTimerId);
        m_nClickDelayTimerId = 0;

        m_nRepeatTimerId = m_pSkin->registerTimerObject(this, 100);
    } else if (nId == m_nRepeatTimerId) {
        switch (m_PushDownPos) {
        case PUSH_DOWN_TOPBT:
            // 滚动条上面的按钮
            {
                if (m_CursorPosLatest == PUSH_DOWN_TOPBT) {
                    // DBG_LOG0("Top button down: Timer...");
                    int nVirtualPosOld = m_nVirtualCurPos;
                    xcreaseVirtualPos(-m_nVirtualLine);
                    if (m_nVirtualCurPos == nVirtualPosOld) {
                        unregisterTimer();
                        return;
                    }

                    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

                    onScroll(SB_LINEUP);

                    invalidate();
                }
            }
            break;
        case PUSH_DOWN_BOTTOMBT:
            // 滚动条下面的按钮
            {
                if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT) {
                    // DBG_LOG0("Bottom button down: Timer...");

                    int nVirtualPosOld = m_nVirtualCurPos;
                    xcreaseVirtualPos(m_nVirtualLine);
                    if (m_nVirtualCurPos == nVirtualPosOld) {
                        unregisterTimer();
                        return;
                    }

                    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

                    onScroll(SB_LINEDOWN);

                    invalidate();
                }
            }
            break;
            // case PUSH_DOWN_THUMB:
            // 滚动条的拖动box(thumb)
            // break;
        case PUSH_DOWN_TOPTRACK:
            // 滚动条的空白位置，进行翻页操作
            {
                if (m_CursorPosLatest == PUSH_DOWN_TOPTRACK) {
                    // DBG_LOG0("Track up : Timer...");

                    int nVirtualPosOld = m_nVirtualCurPos;
                    xcreaseVirtualPos(-m_nVirtualPage);
                    if (m_nVirtualCurPos == nVirtualPosOld) {
                        unregisterTimer();
                        return;
                    }

                    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

                    //
                    // 取得新的位置
                    CPoint pt;

                    m_pSkin->getCursorClientPos(pt);
                    m_CursorPosLatest = getPushDownPos(pt);

                    onScroll(SB_PAGEUP);

                    invalidate();
                }
                break;
            }
        case PUSH_DOWN_BOTTOMTRACK:
            // 滚动条的空白位置，进行翻页操作
            {
                if (m_CursorPosLatest == PUSH_DOWN_BOTTOMTRACK) {
                    // DBG_LOG0("Track down: Timer...");

                    int nVirtualPosOld = m_nVirtualCurPos;
                    xcreaseVirtualPos(m_nVirtualPage);
                    // 按下后的坐标
                    m_nPosThumb = virtualPosToObjectPos(m_nVirtualCurPos);

                    if (m_nVirtualCurPos == nVirtualPosOld) {
                        unregisterTimer();
                        return;
                    }

                    //
                    // 取得新的位置
                    CPoint pt;

                    m_pSkin->getCursorClientPos(pt);
                    m_CursorPosLatest = getPushDownPos(pt);

                    onScroll(SB_PAGEDOWN);

                    invalidate();
                }
            }
            break;
        default:
            break;
        }
    }
}

void CSkinSrollBarBase::registerTimer() {
    m_nClickDelayTimerId = m_pSkin->registerTimerObject(this, 300);
    m_nRepeatTimerId = 0;
}

void CSkinSrollBarBase::unregisterTimer() {
    if (m_nClickDelayTimerId) {
        m_pSkin->unregisterTimerObject(this, m_nClickDelayTimerId);
        m_nClickDelayTimerId = 0;
    }
    if (m_nRepeatTimerId) {
        m_pSkin->unregisterTimerObject(this, m_nRepeatTimerId);
        m_nRepeatTimerId = 0;
    }
}



UIOBJECT_CLASS_NAME_IMP(CSkinVScrollBar, "VScrollBar")

CSkinVScrollBar::CSkinVScrollBar() {
    // 基本设置
    // m_nSBWidth = 15;            // 宽度
    // m_nHeight = 17 * 2 + 31;            // 高度
    m_nHeightPushBt = 17; // 上、下按钮的高度
    m_nHeightThumb = 41; // 滚动条的高度
    m_nHeightTrack = 56;
    m_nSBWidth = 15;
}

CSkinVScrollBar::~CSkinVScrollBar() {
}

void CSkinVScrollBar::draw(CRawGraph *canvas) {
    int y;
    CSFImage *pImg;

    if (m_strBmpFile.size() <= 0) {
        return;
    }

    // 1、绘窗口上面的按钮
    if (m_PushDownPos != PUSH_DOWN_TOPBT) {
        if (m_CursorPosLatest == PUSH_DOWN_TOPBT) {
            pImg = &m_imgTopBtFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgTopBtNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_TOPBT) {
            pImg = &m_imgTopBtPushDown; // 按下状态
        } else {
            pImg = &m_imgTopBtNormal; // 普通
        }
    }

    y = m_rcObj.top;
    pImg->blt(canvas, m_rcObj.left, y);
    y += m_nHeightPushBt;


    // 2、绘窗口中的空白区域
    if (m_nPosThumb > 0) {
        m_imgTrack.yTileBlt(canvas, m_rcObj.left, y,
            m_rcObj.width(), m_nPosThumb);
        y += m_nPosThumb;
    }


    // 3、绘窗口中间的 Thumb
    if (m_PushDownPos != PUSH_DOWN_THUMB) {
        if (m_CursorPosLatest == PUSH_DOWN_THUMB) {
            pImg = &m_imgThumbFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgThumbNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_THUMB) {
            pImg = &m_imgThumbPushDown; // 按下状态
        } else {
            pImg = &m_imgThumbNormal; // 普通
        }
    }

    if (m_bStretchedThumb) {
        pImg->blt(canvas, m_rcObj.left, y, pImg->m_cx, MARGIN_THUMB, pImg->m_x, pImg->m_y);
        pImg->stretchBlt(canvas, m_rcObj.left, y + MARGIN_THUMB, pImg->m_cx, m_nSizeThumbStretched - MARGIN_THUMB * 2, pImg->m_x, pImg->m_y + MARGIN_THUMB, pImg->m_cx, pImg->m_cy - MARGIN_THUMB * 2);
        pImg->blt(canvas, m_rcObj.left, y + m_nSizeThumbStretched - MARGIN_THUMB, pImg->m_cx, MARGIN_THUMB, pImg->m_x, pImg->m_y + pImg->m_cy - MARGIN_THUMB);
    } else {
        pImg->blt(canvas, m_rcObj.left, y);
    }
    y += m_nSizeThumbStretched;


    // 4、绘窗口中的空白区域
    if (m_rcObj.bottom - y - m_nHeightPushBt > 0) {
        m_imgTrack.yTileBlt(canvas, m_rcObj.left, y,
            m_rcObj.width(), m_rcObj.bottom - y - m_nHeightPushBt);
        y = m_rcObj.top + m_rcObj.height() - m_nHeightPushBt;
    }

    // 4、绘窗口下面的按钮
    if (m_PushDownPos != PUSH_DOWN_BOTTOMBT) {
        if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT) {
            pImg = &m_imgBottomBtFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgBottomBtNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT) {
            pImg = &m_imgBottomBtPushDown; // 按下状态
        } else {
            pImg = &m_imgBottomBtNormal; // 普通
        }
    }

    pImg->blt(canvas, m_rcObj.left, y);
}

bool CSkinVScrollBar::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "PushBtHeight") == 0) {
        m_nHeightPushBt = atoi(szValue);
    } else if (strcasecmp(szProperty, "ThumbHeight") == 0) {
        m_nSizeThumbStretched = m_nHeightThumb = atoi(szValue);
    } else if (strcasecmp(szProperty, "ScalableThumb") == 0) {
        m_bStretchedThumb = isTRUE(szValue);
    } else if (strcasecmp(szProperty, "TrackHeight") == 0) {
        m_nHeightTrack = atoi(szValue);
    } else if (strcasecmp(szProperty, "ScrollBarWidth") == 0) {
        m_nSBWidth = atoi(szValue);
    } else if (strcasecmp(szProperty, "LinesPerWheel") == 0) {
        m_nLinesPerWheel = atoi(szValue);
    } else if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        // 图片
        m_strBmpFile = szValue;

        m_imgTopBtNormal.loadFromSRM(m_pSkin, szValue);
        m_imgTopBtFocus.loadFromSRM(m_pSkin, szValue);
        m_imgTopBtPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgBottomBtNormal.loadFromSRM(m_pSkin, szValue);
        m_imgBottomBtFocus.loadFromSRM(m_pSkin, szValue);
        m_imgBottomBtPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgThumbNormal.loadFromSRM(m_pSkin, szValue);
        m_imgThumbFocus.loadFromSRM(m_pSkin, szValue);
        m_imgThumbPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgTrack.loadFromSRM(m_pSkin, szValue);

        // 根据前面设置的
        // 上面的按钮
        m_imgTopBtNormal.m_x = 0;
        m_imgTopBtPushDown.m_x = m_nSBWidth;
        m_imgTopBtFocus.m_x = m_nSBWidth * 2;
        m_imgTopBtPushDown.m_y = m_imgTopBtFocus.m_y = m_imgTopBtNormal.m_y = 0;
        m_imgTopBtPushDown.m_cx = m_imgTopBtFocus.m_cx = m_imgTopBtNormal.m_cx = m_nSBWidth;
        m_imgTopBtPushDown.m_cy = m_imgTopBtFocus.m_cy = m_imgTopBtNormal.m_cy = m_nHeightPushBt;

        // 下面的按钮
        m_imgBottomBtNormal.m_x = 0;
        m_imgBottomBtPushDown.m_x = m_nSBWidth;
        m_imgBottomBtFocus.m_x = m_nSBWidth * 2;
        m_imgBottomBtPushDown.m_y = m_imgBottomBtFocus.m_y = m_imgBottomBtNormal.m_y = m_nHeightPushBt + m_nHeightThumb + m_nHeightTrack;
        m_imgBottomBtPushDown.m_cx = m_imgBottomBtFocus.m_cx = m_imgBottomBtNormal.m_cx = m_nSBWidth;
        m_imgBottomBtPushDown.m_cy = m_imgBottomBtFocus.m_cy = m_imgBottomBtNormal.m_cy = m_nHeightPushBt;

        // 拖动的thumb
        m_imgThumbNormal.m_x = 0;
        m_imgThumbPushDown.m_x = m_nSBWidth;
        m_imgThumbFocus.m_x = m_nSBWidth * 2;
        m_imgThumbPushDown.m_y = m_imgThumbFocus.m_y = m_imgThumbNormal.m_y = m_nHeightPushBt;
        m_imgThumbPushDown.m_cx = m_imgThumbFocus.m_cx = m_imgThumbNormal.m_cx = m_nSBWidth;
        m_imgThumbPushDown.m_cy = m_imgThumbFocus.m_cy = m_imgThumbNormal.m_cy = m_nHeightThumb;

        // 空白的轨道Track
        m_imgTrack.m_x = 0;
        m_imgTrack.m_y = m_nHeightPushBt + m_nHeightThumb;
        m_imgTrack.m_cx = m_nSBWidth;
        m_imgTrack.m_cy = m_nHeightTrack;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinVScrollBar::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropInt("PushBtHeight", m_nHeightPushBt);
    listProperties.addPropInt("ThumbHeight", m_nHeightThumb);
    listProperties.addPropBoolStr("ScalableThumb", m_bStretchedThumb, m_bStretchedThumb);
    listProperties.addPropInt("TrackHeight", m_nHeightTrack);
    listProperties.addPropInt("ScrollBarWidth", m_nSBWidth);

    listProperties.addPropImageFile(SZ_PN_IMAGE, m_strBmpFile.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinVScrollBar::onScroll(uint32_t nSBCode) {
    if (m_pScrollNotify) {
        m_pScrollNotify->onVScroll(nSBCode, getScrollPos(), this);
    } else {
        m_pSkin->onVScroll(nSBCode, getScrollPos(), this);
    }
}

void CSkinVScrollBar::adjustThumbSize() {
    if (m_bStretchedThumb) {
        // Thumb is stretched?
        if (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage == 0 || m_nVirtualMax - m_nVirtualMin <= 0) {
            m_nSizeThumbStretched = m_rcObj.height() - m_nHeightPushBt * 2;
        } else {
            m_nSizeThumbStretched = (m_rcObj.height() - m_nHeightPushBt * 2) * m_nVirtualPage / (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage);
        }
        if (m_nSizeThumbStretched < MARGIN_THUMB * 2) {
            m_nSizeThumbStretched = MARGIN_THUMB * 2;
        }
    } else {
        m_nSizeThumbStretched = m_nHeightThumb;
    }
}

CSkinVScrollBar::PUSH_DOWN_POS CSkinVScrollBar::getPushDownPos(CPoint pt) {
    //assert(isPtIn(pt));
    if (!isPtIn(pt)) {
        return PUSH_DOWN_NONE;
    }

    int y;

    y = m_rcObj.top + m_nHeightPushBt;
    if (pt.y < y) {
        return PUSH_DOWN_TOPBT;
    }

    y += m_nPosThumb;
    if (pt.y < y) {
        return PUSH_DOWN_TOPTRACK;
    }

    y += m_nSizeThumbStretched;
    if (pt.y < y) {
        return PUSH_DOWN_THUMB;
    }

    y = m_rcObj.bottom - m_nHeightPushBt;
    if (pt.y < y) {
        return PUSH_DOWN_BOTTOMTRACK;
    }

    return PUSH_DOWN_BOTTOMBT;
}

void CSkinVScrollBar::thumbOnLButtonDown(uint32_t nFlags, CPoint point) {
    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

    // DBG_LOG0("Thumb button down");

    m_nCursorToThumbBeg = point.y - (m_rcObj.top + m_nHeightPushBt + m_nPosThumb);

    invalidate();
}

void CSkinVScrollBar::thumbOnMouseDrag(CPoint point) {
    int nVirtualPosOld = m_nVirtualCurPos, nPosThumbOld = m_nPosThumb;

    m_nPosThumb = point.y - (m_rcObj.top + m_nHeightPushBt) - m_nCursorToThumbBeg;

    if (m_nPosThumb < 0) {
        m_nPosThumb = 0;
    } else if (m_nPosThumb > m_rcObj.height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched) {
        m_nPosThumb = m_rcObj.height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched;
    }

    // // DBG_LOG1("m_nPosThumb: %d", m_nPosThumb);

    m_nVirtualCurPos = objectPosToVirtualPos(m_nPosThumb);

    if (m_nVirtualCurPos == nVirtualPosOld) {
        if (m_nPosThumb != nPosThumbOld) {
            invalidate();
        }
        return;
    }

    onScroll(SB_THUMBPOSITION);

    invalidate();
}

int CSkinVScrollBar::virtualPosToObjectPos(int nVirtualPos) {
    // assert(m_nVirtualMax - m_nVirtualMin != 0);

    if (m_nVirtualMax - m_nVirtualMin == 0) {
        return 0;
    }

    return int((double)nVirtualPos * (m_rcObj.height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched)) / (m_nVirtualMax - m_nVirtualMin);
}

int CSkinVScrollBar::objectPosToVirtualPos(int nThumbPos) {
    if (m_rcObj.height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched == 0) {
        return 0;
    }

    return int((double)nThumbPos * (m_nVirtualMax - m_nVirtualMin) / (m_rcObj.height() - m_nHeightPushBt * 2 - m_nSizeThumbStretched));
}

//////////////////////////////////////////////////////////////////////
// CSkinHScrollBar
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinHScrollBar, "HScrollBar")

CSkinHScrollBar::CSkinHScrollBar() {
    m_nWidthPushBt = 17; // 上、下按钮的高度
    m_nWidthThumb = 41; // 滚动条的高度
    m_nWidthTrack = 56;
    m_nSBHeight = 15;
}

CSkinHScrollBar::~CSkinHScrollBar() {
}

void CSkinHScrollBar::draw(CRawGraph *canvas) {
    int x;
    CSFImage *pImg;

    if (m_strBmpFile.size() <= 0) {
        return;
    }

    // 1、绘窗口上面的按钮
    if (m_PushDownPos != PUSH_DOWN_TOPBT) {
        if (m_CursorPosLatest == PUSH_DOWN_TOPBT) {
            pImg = &m_imgTopBtFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgTopBtNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_TOPBT) {
            pImg = &m_imgTopBtPushDown; // 按下状态
        } else {
            pImg = &m_imgTopBtNormal; // 普通
        }
    }

    x = m_rcObj.left;
    pImg->blt(canvas, x, m_rcObj.top);
    x += m_nWidthPushBt;


    // 2、绘窗口中的空白区域
    if (m_nPosThumb > 0) {
        m_imgTrack.xTileBlt(canvas, x, m_rcObj.top,
            m_nPosThumb, m_rcObj.height());
        x += m_nPosThumb;
    }


    // 3、绘窗口中间的 Thumb
    if (m_PushDownPos != PUSH_DOWN_THUMB) {
        if (m_CursorPosLatest == PUSH_DOWN_THUMB) {
            pImg = &m_imgThumbFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgThumbNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_THUMB) {
            pImg = &m_imgThumbPushDown; // 按下状态
        } else {
            pImg = &m_imgThumbNormal; // 普通
        }
    }

    if (m_bStretchedThumb) {
        pImg->blt(canvas, x, m_rcObj.top, MARGIN_THUMB, pImg->m_cy, pImg->m_x, pImg->m_y);
        pImg->stretchBlt(canvas, x + MARGIN_THUMB, m_rcObj.top, m_nSizeThumbStretched - MARGIN_THUMB * 2, pImg->m_cy, pImg->m_x + MARGIN_THUMB, pImg->m_y, pImg->m_cx - MARGIN_THUMB * 2, pImg->m_cy);
        pImg->blt(canvas, x + m_nSizeThumbStretched - MARGIN_THUMB, m_rcObj.top, MARGIN_THUMB, pImg->m_cy, pImg->m_x + pImg->m_cx - MARGIN_THUMB, pImg->m_y);
    } else {
        pImg->blt(canvas, x, m_rcObj.top);
    }
    x += m_nSizeThumbStretched;


    // 4、绘窗口中的空白区域
    if (m_rcObj.right - x - m_nWidthPushBt > 0) {
        m_imgTrack.xTileBlt(canvas, x, m_rcObj.top,
            m_rcObj.right - x - m_nWidthPushBt, m_rcObj.height());
        x = m_rcObj.right - m_nWidthPushBt;
    }

    // 4、绘窗口下面的按钮
    if (m_PushDownPos != PUSH_DOWN_BOTTOMBT) {
        if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT) {
            pImg = &m_imgBottomBtFocus; // 光标 hover 在上面的按钮上
        } else {
            pImg = &m_imgBottomBtNormal; // 普通
        }
    } else {
        if (m_CursorPosLatest == PUSH_DOWN_BOTTOMBT) {
            pImg = &m_imgBottomBtPushDown; // 按下状态
        } else {
            pImg = &m_imgBottomBtNormal; // 普通
        }
    }

    pImg->blt(canvas, x, m_rcObj.top);
}

bool CSkinHScrollBar::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CUIObject::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "PushBtWidth") == 0) {
        m_nWidthPushBt = atoi(szValue);
    } else if (strcasecmp(szProperty, "ThumbWidth") == 0) {
        m_nSizeThumbStretched = m_nWidthThumb = atoi(szValue);
    } else if (strcasecmp(szProperty, "ScalableThumb") == 0) {
        m_bStretchedThumb = isTRUE(szValue);
    } else if (strcasecmp(szProperty, "TrackWidth") == 0) {
        m_nWidthTrack = atoi(szValue);
    } else if (strcasecmp(szProperty, "ScrollBarHeight") == 0) {
        m_nSBHeight = atoi(szValue);
    } else if (strcasecmp(szProperty, "LinesPerWheel") == 0) {
        m_nLinesPerWheel = atoi(szValue);
    } else if (strcasecmp(szProperty, SZ_PN_IMAGE) == 0) {
        // 图片
        m_strBmpFile = szValue;

        m_imgTopBtNormal.loadFromSRM(m_pSkin, szValue);
        m_imgTopBtFocus.loadFromSRM(m_pSkin, szValue);
        m_imgTopBtPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgBottomBtNormal.loadFromSRM(m_pSkin, szValue);
        m_imgBottomBtFocus.loadFromSRM(m_pSkin, szValue);
        m_imgBottomBtPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgThumbNormal.loadFromSRM(m_pSkin, szValue);
        m_imgThumbFocus.loadFromSRM(m_pSkin, szValue);
        m_imgThumbPushDown.loadFromSRM(m_pSkin, szValue);

        m_imgTrack.loadFromSRM(m_pSkin, szValue);

        // 根据前面设置的
        // 上面的按钮
        m_imgTopBtNormal.m_y = 0;
        m_imgTopBtPushDown.m_y = m_nSBHeight;
        m_imgTopBtFocus.m_y = m_nSBHeight * 2;
        m_imgTopBtPushDown.m_x = m_imgTopBtFocus.m_x = m_imgTopBtNormal.m_x = 0;
        m_imgTopBtPushDown.m_cy = m_imgTopBtFocus.m_cy = m_imgTopBtNormal.m_cy = m_nSBHeight;
        m_imgTopBtPushDown.m_cx = m_imgTopBtFocus.m_cx = m_imgTopBtNormal.m_cx = m_nWidthPushBt;

        // 下面的按钮
        m_imgBottomBtNormal.m_y = 0;
        m_imgBottomBtPushDown.m_y = m_nSBHeight;
        m_imgBottomBtFocus.m_y = m_nSBHeight * 2;
        m_imgBottomBtPushDown.m_x = m_imgBottomBtFocus.m_x = m_imgBottomBtNormal.m_x = m_nWidthPushBt + m_nWidthThumb + m_nWidthTrack;
        m_imgBottomBtPushDown.m_cy = m_imgBottomBtFocus.m_cy = m_imgBottomBtNormal.m_cy = m_nSBHeight;
        m_imgBottomBtPushDown.m_cx = m_imgBottomBtFocus.m_cx = m_imgBottomBtNormal.m_cx = m_nWidthPushBt;

        // 拖动的thumb
        m_imgThumbNormal.m_y = 0;
        m_imgThumbPushDown.m_y = m_nSBHeight;
        m_imgThumbFocus.m_y = m_nSBHeight * 2;
        m_imgThumbPushDown.m_x = m_imgThumbFocus.m_x = m_imgThumbNormal.m_x = m_nWidthPushBt;
        m_imgThumbPushDown.m_cy = m_imgThumbFocus.m_cy = m_imgThumbNormal.m_cy = m_nSBHeight;
        m_imgThumbPushDown.m_cx = m_imgThumbFocus.m_cx = m_imgThumbNormal.m_cx = m_nWidthThumb;

        // 空白的轨道Track
        m_imgTrack.m_y = 0;
        m_imgTrack.m_x = m_nWidthPushBt + m_nWidthThumb;
        m_imgTrack.m_cy = m_nSBHeight;
        m_imgTrack.m_cx = m_nWidthTrack;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinHScrollBar::enumProperties(CUIObjProperties &listProperties) {
    CUIObject::enumProperties(listProperties);

    listProperties.addPropInt("PushBtWidth", m_nWidthPushBt);
    listProperties.addPropInt("ThumbWidth", m_nWidthThumb);
    listProperties.addPropBoolStr("ScalableThumb", m_bStretchedThumb, m_bStretchedThumb);
    listProperties.addPropInt("TrackWidth", m_nWidthTrack);
    listProperties.addPropInt("ScrollBarHeight", m_nSBHeight);

    listProperties.addPropImageFile(SZ_PN_IMAGE, m_strBmpFile.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinHScrollBar::onScroll(uint32_t nSBCode) {
    if (m_pScrollNotify) {
        m_pScrollNotify->onHScroll(nSBCode, getScrollPos(), this);
    } else {
        m_pSkin->onHScroll(nSBCode, getScrollPos(), this);
    }
}

void CSkinHScrollBar::adjustThumbSize() {
    if (m_bStretchedThumb) {
        // Thumb is stretched?
        if (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage == 0 || m_nVirtualMax - m_nVirtualMin <= 0) {
            m_nSizeThumbStretched = m_rcObj.width() - m_nWidthPushBt * 2;
        } else {
            m_nSizeThumbStretched = (m_rcObj.width() - m_nWidthPushBt * 2) * m_nVirtualPage / (m_nVirtualMax - m_nVirtualMin + m_nVirtualPage);
        }
        if (m_nSizeThumbStretched < MARGIN_THUMB * 2) {
            m_nSizeThumbStretched = MARGIN_THUMB * 2;
        }
    } else {
        m_nSizeThumbStretched = m_nWidthThumb;
    }
}

CSkinHScrollBar::PUSH_DOWN_POS CSkinHScrollBar::getPushDownPos(CPoint pt) {
    //assert(isPtIn(pt));
    if (!isPtIn(pt)) {
        return PUSH_DOWN_NONE;
    }

    int x;

    x = m_rcObj.left + m_nWidthPushBt;
    if (pt.x < x) {
        return PUSH_DOWN_TOPBT;
    }

    x += m_nPosThumb;
    if (pt.x < x) {
        return PUSH_DOWN_TOPTRACK;
    }

    x += m_nSizeThumbStretched;
    if (pt.x < x) {
        return PUSH_DOWN_THUMB;
    }

    x = m_rcObj.right - m_nWidthPushBt;
    if (pt.x < x) {
        return PUSH_DOWN_BOTTOMTRACK;
    }

    return PUSH_DOWN_BOTTOMBT;
}

void CSkinHScrollBar::thumbOnLButtonDown(uint32_t nFlags, CPoint point) {
    // 捕捉鼠标输入
    m_pSkin->setCaptureMouse(this);

    // DBG_LOG0("Thumb button down");

    m_nCursorToThumbBeg = point.x - (m_rcObj.left + m_nWidthPushBt + m_nPosThumb);

    invalidate();
}

void CSkinHScrollBar::thumbOnMouseDrag(CPoint point) {
    // DBG_LOG2("Drag Thumb OK! x: %d, y: %d", point.x, point.y);

    int nVirtualPosOld = m_nVirtualCurPos, nPosThumbOld = m_nPosThumb;

    m_nPosThumb = point.x - (m_rcObj.left + m_nWidthPushBt) - m_nCursorToThumbBeg;

    if (m_nPosThumb < 0) {
        m_nPosThumb = 0;
    } else if (m_nPosThumb > m_rcObj.width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched) {
        m_nPosThumb = m_rcObj.width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched;
    }

    m_nVirtualCurPos = objectPosToVirtualPos(m_nPosThumb);

    if (m_nVirtualCurPos == nVirtualPosOld) {
        if (m_nPosThumb != nPosThumbOld) {
            invalidate();
        }
        return;
    }

    onScroll(SB_THUMBPOSITION);

    invalidate();
}

int CSkinHScrollBar::virtualPosToObjectPos(int nVirtualPos) {
    // assert(m_nVirtualMax - m_nVirtualMin != 0);

    if (m_nVirtualMax - m_nVirtualMin == 0) {
        return 0;
    }

    return int((double)nVirtualPos * (m_rcObj.width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched)) / (m_nVirtualMax - m_nVirtualMin);
}

int CSkinHScrollBar::objectPosToVirtualPos(int nThumbPos) {
    if (m_rcObj.width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched == 0) {
        return 0;
    }

    return int((double)nThumbPos * (m_nVirtualMax - m_nVirtualMin) / (m_rcObj.width() - m_nWidthPushBt * 2 - m_nSizeThumbStretched));
}
