// UIEditObject.h: interface for the CUIEditObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
#define AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _OUR_MFC_LIB

// typedef vector<string>    CProperties;

class CUIObject;
interface ISESkinNotify;

template<class _BaseUIObjClass>
class CSEUIEditObject : public _BaseUIObjClass
{
public:
    CSEUIEditObject(ISESkinNotify *pNotify)
    {
        m_bLBtDown = false;
        static uint8_t    r = 0, g = 0, b = 0;
        r += 75;
        g += 50;
        b += 25;
        m_brBk.CreateSolidBrush(CColor(RGB(r, g, b)));
        m_msgNeed = UO_MSG_WANT_ALL;
        m_pNotify = pNotify;
    }
    virtual ~CSEUIEditObject()
    {
    }

    virtual bool onLButtonDown(uint32_t nFlags, CPoint point)
    {
        m_bLBtDown = true;
        m_bCanBeMoved = false;
        m_ptDragOld = point;

        // 捕捉鼠标输入
        m_pSkin->setCaptureMouse(this);

        return true;
    }
    virtual bool onLButtonUp(uint32_t nFlags, CPoint point)
    {
        m_bLBtDown = false;
        m_bCanBeMoved = false;

        // 释放鼠标输入
        m_pSkin->releaseCaptureMouse(this);
        return true;
    }
    virtual bool onMouseMove(uint32_t nFlags, CPoint point)
    {
        if (m_bLBtDown)
        {
            //
            // 移动此控件
            CString        str;
            // int            nNewPos;
            int            nOffset;

            if (!m_bCanBeMoved)
            {
                if (abs(point.x - m_ptDragOld.x) < 4 && abs(point.y - m_ptDragOld.y) < 4)
                    return true;
                else
                    m_bCanBeMoved = true;
            }

            nOffset = point.x - m_ptDragOld.x;
            // nNewPos = m_rcObj.left + point.x - m_ptDragOld.x + SIZE_MARGIN;
            if (nOffset != 0)
            {
                if (nOffset > 0)
                    str.Format("%s + %d", m_formLeft.getFormula(), nOffset);
                else
                    str.Format("%s%d", m_formLeft.getFormula(), nOffset);
                setProperty("left", str);

                if (m_pNotify)
                    m_pNotify->onFocusUIObjPropertyChanged("left", m_formLeft.getFormula());
            }

            nOffset = point.y - m_ptDragOld.y;
            // nNewPos = m_rcObj.top + point.y - m_ptDragOld.y + SIZE_MARGIN;
            if (nOffset != 0)
            {
                if (nOffset > 0)
                    str.Format("%s + %d", m_formTop.getFormula(), nOffset);
                else
                    str.Format("%s%d", m_formTop.getFormula(), nOffset);
                setProperty("top", str);
                if (m_pNotify)
                    m_pNotify->onFocusUIObjPropertyChanged("top", m_formTop.getFormula());
            }

            m_pSkin->RecalculateSizePos();

            m_pSkin->invalidateRect(nullptr, true);

            m_ptDragOld = point;
        }
        return true;
    }

    virtual void draw(CRawGraph *canvas)
    {
        canvas->fillRect(m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), &m_brBk);

        _BaseUIObjClass::draw(canvas);

        if (isOnFocus())
        {
        //    BitBlt(hdc, m_rcObj.left, m_rcObj.top, m_rcObj.width(), m_rcObj.height(), nullptr, 0, 0, WHITENESS);
            HDC        hdc = canvas->getHandle();

            HPEN    hPen, hPenOld;
            COLORREF    clrOld;
            int            nBkModeOld;

            hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
            hPenOld = (HPEN)SelectObject(hdc, hPen);
            clrOld = ::setBkColor(hdc, RGB(255, 255, 255));
            nBkModeOld = SetBkMode(hdc, OPAQUE);

            // 画边缘
            // for (int i = 0; i < SIZE_MARGIN; i+=2)
            int    i = 0;
            {
                MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, nullptr);
                LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.top + i);

                MoveToEx(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1, nullptr);
                LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);

                MoveToEx(hdc, m_rcObj.left + i, m_rcObj.top + i, nullptr);
                LineTo(hdc, m_rcObj.left + i, m_rcObj.bottom - i - 1);

                MoveToEx(hdc, m_rcObj.right - i - 1, m_rcObj.top + i, nullptr);
                LineTo(hdc, m_rcObj.right - i - 1, m_rcObj.bottom - i - 1);
            }

            SetBkMode(hdc, nBkModeOld);
            ::setBkColor(hdc, clrOld);
            SelectObject(hdc, hPenOld);

            DeleteObject(hPen);
        }
    }

    virtual bool setProperty(cstr_t szProperty, cstr_t szValue)
    {
        if (strcasecmp(szProperty, SZ_PN_NAME) == 0)
        {
            _BaseUIObjClass::setProperty(szProperty, szValue);

            if (m_pNotify)
                m_pNotify->onUIObjNameChanged(this);

            return true;
        }

        return _BaseUIObjClass::setProperty(szProperty, szValue);
    }

    void setNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

protected:
    virtual void onKillFocus()
    {
        _BaseUIObjClass::onKillFocus();
        m_pSkin->invalidateRect();
    }
    virtual void onSetFocus()
    {
        _BaseUIObjClass::onSetFocus();
        invalidate();
    }

protected:
    bool            m_bLBtDown;
    CPoint            m_ptDragOld;            // drag begin pos
    bool            m_bCanBeMoved;

    CMLBrush        m_brBk;

    ISESkinNotify    *m_pNotify;

};

#endif // !defined(AFX_SEUIEDITOBJECT_H__40C341C2_8FA6_4486_8396_5A8541E09C9E__INCLUDED_)
