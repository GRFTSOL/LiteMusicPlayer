// SESkinContainer.h: interface for the CSESkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_)
#define AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_

#define INDEX_FOCUS_SELF        -2
interface ISESkinNotify;

template<class _BaseSkinContainer>
class CSESkinContainer : public _BaseSkinContainer
{
public:
    CSESkinContainer(ISESkinNotify *pNotify)
    {
        m_pNotify = pNotify;
    }
    virtual ~CSESkinContainer()
    {

    }

    void onKillFocus()
    {
        m_pSkin->invalidateRect(nullptr, true);
    }
    void onSetFocus()
    {
        invalidate();
    }

    void draw(CRawGraph *canvas)
    {
        HDC        hdc = canvas->getHandle();

        CRect    rc;
        rc = m_rcObj;

        _BaseSkinContainer::draw(canvas);

        if (isOnFocus())
        {
            HPEN    hPen, hPenOld;
            COLORREF    clrOld;
            int            nBkModeOld;

            hPen = CreatePen(PS_DOT, 1, RGB(0x31, 0x69, 0xc6));
            hPenOld = (HPEN)SelectObject(hdc, hPen);
            clrOld = setBkColor(hdc, RGB(255, 255, 255));
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
            setBkColor(hdc, clrOld);
            SelectObject(hdc, hPenOld);

            DeleteObject(hPen);
        }
    }

    bool setProperty(cstr_t szProperty, cstr_t szValue)
    {
        bool    bRet = _BaseSkinContainer::setProperty(szProperty, szValue);

        if (strcasecmp(szProperty, SZ_PN_NAME) == 0)
        {
            if (m_pNotify)
                m_pNotify->onUIObjNameChanged(this);

            return true;
        }

        return bRet;
    }

    void setNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }

protected:
    void switchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew)
    {
        if (pUIObjFocusOld != pUIObjFocusNew)
        {
            if (pUIObjFocusOld)
                pUIObjFocusOld->onKillFocus();

            if (pUIObjFocusNew)
                pUIObjFocusNew->onSetFocus();
        }
    }


    ISESkinNotify    *m_pNotify;

};

#endif // !defined(AFX_SESKINCONTAINER_H__936D3562_73AB_43F1_B099_A2B928C3A45B__INCLUDED_)
