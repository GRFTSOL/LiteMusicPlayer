// ImageRectSel.cpp : implementation file
//

#include "MPSkinEditor.h"
#include "ImageRectSel.h"
#include "../gfx/RawImageUtils.h"

CImageSelRectEx::CImageSelRectEx()
{
    m_nScale = 2;
    m_bLBtDown = false;
    m_bDragAll = false;
    m_nSelVertex = -1;
}

CImageSelRectEx::~CImageSelRectEx()
{
    close();
}

void CImageSelRectEx::close()
{
    m_nSelVertex = -1;
    for (int i = 0; i < m_vVertexOBX.size(); i++)
    {
        Vertex    *pV = m_vVertexOBX[i];
        delete pV;
    }
    m_vVertexOBX.clear();
    m_vLine.clear();
    m_vGroup.clear();
}

void CImageSelRectEx::addVertexOrderByX(Vertex *pVer)
{
    m_vVertexOBX.push_back(pVer);
}

void CImageSelRectEx::addVertexOrderByY(Vertex *pVer)
{
    m_vVertexOBY.push_back(pVer);
}

void CImageSelRectEx::addLine(Line &line)
{
    m_vLine.push_back(line);
}

void CImageSelRectEx::addLine(Vertex *pV1, Vertex *pV2)
{
    Line    line;
    line.pV1 = pV1;
    line.pV2 = pV2;

    m_vLine.push_back(line);
}

void CImageSelRectEx::addGroup(Group &group)
{
    m_vGroup.push_back(group);
}

void CImageSelRectEx::setSelAllRect(Vertex *pV1, Vertex *pV2)
{
    m_SelAllRect.vVertex.push_back(pV1);
    m_SelAllRect.vVertex.push_back(pV2);
}

int getPtDistance(int x1, int y1, int x2, int y2)
{
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

void CImageSelRectEx::onLButtonDown(uint32_t nFlags, CPoint point)
{
    CRect    rc;
    int        nNearGroupDistance = 1024;
    int        nSelVertext = -1;
    Vertex    *pVerSel = nullptr;

    for (int i = 0; i < m_vVertexOBX.size(); i++)
    {
        Vertex    *pV = m_vVertexOBX[i];

        rc.setLTRB(pV->x - 3, pV->y - 3, pV->x + 3, pV->y + 3);
        if (rc.ptInRect(point))
        {
            int        n = getPtDistance(pV->x, pV->y, point.x, point.y);
            if (n < nNearGroupDistance)
            {
                nNearGroupDistance = n;
                pVerSel = pV;
                nSelVertext = i;
            }
        }
    }

    if (nSelVertext != m_nSelVertex)
    {
        HDC hdc = ::GetDC(m_hWnd);
        drawXor(hdc);
        m_nSelVertex = nSelVertext;
        drawXor(hdc);
        ::ReleaseDC(m_hWnd, hdc);
    }

    if (m_nSelVertex == -1 && m_SelAllRect.vVertex.size() == 2)
    {
        // Is Select all?
        CRect        rc;
        rc.setLTRB(m_SelAllRect.vVertex[0]->x, m_SelAllRect.vVertex[0]->y,
            m_SelAllRect.vVertex[1]->x, m_SelAllRect.vVertex[1]->y);
        if (rc.ptInRect(point))
            m_bDragAll = true;
    }

    m_bLBtDown = true;
    m_ptDragOld = point;
}

void CImageSelRectEx::onLButtonUp(uint32_t nFlags, CPoint point)
{
    m_bLBtDown = false;
    m_bDragAll = false;
}

CImageSelRectEx::Group *CImageSelRectEx::findGroup(Vertex *pV, bool bVert)
{
    for (int i = 0; i < m_vGroup.size(); i++)
    {
        Group    &gr = m_vGroup[i];
        if (gr.bVert != bVert)
            continue;

        for (int k = 0; k < gr.vVertex.size(); k++)
        {
            Vertex    *pT = gr.vVertex[k];
            if (pT == pV)
            {
                // find the group
                return &gr;
            }
        }
    }

    return nullptr;
}

void CImageSelRectEx::offsetVertexInGroup(Group *gr, bool bVert, int nOffset)
{
    assert(gr->bVert == bVert);

    if (bVert)
    {
        for (int k = 0; k < gr->vVertex.size(); k++)
        {
            Vertex    *pT = gr->vVertex[k];
            pT->y += nOffset;
        }
    }
    else
    {
        // horz
        for (int k = 0; k < gr->vVertex.size(); k++)
        {
            Vertex    *pT = gr->vVertex[k];
            pT->x += nOffset;
        }
    }
}

void CImageSelRectEx::offsetVertex(int nXOffset, int nYOffset)
{
    if (m_nSelVertex != -1)
    {
        assert(m_nSelVertex >= 0 && m_nSelVertex < m_vVertexOBX.size());

        Vertex    *pV = m_vVertexOBX[m_nSelVertex];

        if (nXOffset != 0 && pV->bCanMoveHorz)
        {
            // move all vertex in group
            Group    *gr = findGroup(pV, false);
            if (gr)
                offsetVertexInGroup(gr, false, nXOffset);
            else
                pV->x += nXOffset;    // The vertex doesn't belong to any group
        }

        if (nYOffset != 0 && pV->bCanMoveVert)
        {
            // move all vertex in group
            Group    *gr = findGroup(pV, true);
            if (gr)
                offsetVertexInGroup(gr, true, nYOffset);
            else
                pV->y += nYOffset;    // The vertex doesn't belong to any group
        }
    }
    else if (m_bDragAll)
    {
        for (int i = 0; i < m_vVertexOBX.size(); i++)
        {
            Vertex    *pV = m_vVertexOBX[i];

            pV->x += nXOffset;

            pV->y += nYOffset;
        }
    }
}

void CImageSelRectEx::offsetAllVertex(int nXOffset, int nYOffset)
{
    for (int i = 0; i < m_vVertexOBX.size(); i++)
    {
        Vertex    *pV = m_vVertexOBX[i];

        pV->x += nXOffset;

        pV->y += nYOffset;
    }
}

void CImageSelRectEx::setVertex(Vertex *pV, int x, int y)
{
    if (x != 0)
    {
        // modify x in same group
        Group *gr = findGroup(pV, false);
        for (int i = 0; i < gr->vVertex.size(); i++)
        {
            Vertex    *v = gr->vVertex[i];
            v->x = x;
        }
    }

    if (y != 0)
    {
        // modify y in same group
        Group *gr = findGroup(pV, true);
        for (int i = 0; i < gr->vVertex.size(); i++)
        {
            Vertex    *v = gr->vVertex[i];
            v->y = y;
        }
    }
}

void CImageSelRectEx::onMouseMove(uint32_t nFlags, CPoint point)
{
    if (m_bLBtDown && (m_nSelVertex != -1 || m_bDragAll))
    {
        HDC hdc = ::GetDC(m_hWnd);
        drawXor(hdc);

        offsetVertex(point.x - m_ptDragOld.x, point.y - m_ptDragOld.y);

        drawXor(hdc);
        ::ReleaseDC(m_hWnd, hdc);

        ::sendMessage(::getParent(m_hWnd), WM_COMMAND, IDC_UPDATE_RC, 0);

        m_ptDragOld = point;
    }
}

void CImageSelRectEx::onKeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags)
{
    if (m_bLBtDown)
        return;

    if (nChar == VK_LEFT || nChar == VK_UP || nChar == VK_RIGHT || nChar == VK_DOWN)
    {
        int        nXOffset = 0, nYOffset = 0;

        if (nChar == VK_LEFT || nChar == VK_RIGHT)
        {
            if (nChar == VK_LEFT)
                nXOffset = -1;
            else
                nXOffset = 1;
        }
        else if (nChar == VK_UP || nChar == VK_DOWN)
        {
            if (nChar == VK_UP)
                nYOffset = -1;
            else
                nYOffset = 1;
        }

        HDC hdc = ::GetDC(m_hWnd);
        drawXor(hdc);

        if (m_nSelVertex == -1)
            m_bDragAll = true;
        offsetVertex(nXOffset, nYOffset);
        m_bDragAll = false;

        drawXor(hdc);
        ::ReleaseDC(m_hWnd, hdc);

        ::sendMessage(::getParent(m_hWnd), WM_COMMAND, IDC_UPDATE_RC, 0);
    }
}

void drawVertex(HDC hdc, CImageSelRectEx::Vertex *pV, int nSize)
{
    MoveToEx(hdc, pV->x - nSize, pV->y - nSize, nullptr);
    LineTo(hdc, pV->x + nSize, pV->y - nSize);

    LineTo(hdc, pV->x + nSize, pV->y + nSize);

    LineTo(hdc, pV->x - nSize, pV->y + nSize);

    LineTo(hdc, pV->x - nSize, pV->y - nSize);
}

void CImageSelRectEx::drawXor(HDC hdc)
{
    HPEN        hPen, hPenOld;
    COLORREF    clrOld;
    int            nBkModeOld;
    int            nRop2Old;
    CRect        rc;

    hPen = CreatePen(PS_DOT, 1, RGB(0x7F, 0x7F, 0x7F));
    hPenOld = (HPEN)SelectObject(hdc, hPen);
    clrOld = setBkColor(hdc, RGB(255, 255, 255));
    nBkModeOld = SetBkMode(hdc, OPAQUE);
    nRop2Old = SetROP2(hdc, R2_NOTXORPEN);

    draw(hdc, m_nScale);

    SetBkMode(hdc, nBkModeOld);
    setBkColor(hdc, clrOld);
    SelectObject(hdc, hPenOld);
    SetROP2(hdc, nRop2Old);

    DeleteObject(hPen);
}

void CImageSelRectEx::draw(HDC hdc, int nScale)
{
    uint32_t        i;
    Vertex        v;

    for (i = 0; i < m_vLine.size(); i++)
    {
        MoveToEx(hdc, m_vLine[i].pV1->x * nScale, m_vLine[i].pV1->y * nScale, nullptr);
        LineTo(hdc, m_vLine[i].pV2->x * nScale, m_vLine[i].pV2->y * nScale);
    }

    HPEN        hPen, hPenOld;

    hPen = CreatePen(PS_SOLID, 1, RGB(0x7F, 0x7F, 0x7F));
    hPenOld = (HPEN)SelectObject(hdc, hPen);

    for (i = 0; i < m_vVertexOBX.size(); i++)
    {
        v.x = m_vVertexOBX[i]->x * nScale;
        v.y = m_vVertexOBX[i]->y * nScale;
        drawVertex(hdc, &v, 2);
    }

    // draw focus vertex
    if (m_nSelVertex != -1)
    {
        v.x = m_vVertexOBX[m_nSelVertex]->x * nScale;
        v.y = m_vVertexOBX[m_nSelVertex]->y * nScale;
        drawVertex(hdc, &v, 3);
    }

    SelectObject(hdc, hPenOld);

    DeleteObject(hPen);
}


/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl

CImageRectSelCtrl::CImageRectSelCtrl()
{
    m_nScale = 3;
    m_imgRectSelType = IRST_NONE;

    m_pV1 = new CImageSelRectEx::Vertex;
    m_pV2 = new CImageSelRectEx::Vertex;
    m_pV3 = new CImageSelRectEx::Vertex;
    m_pV4 = new CImageSelRectEx::Vertex;
    m_pVSS = new CImageSelRectEx::Vertex;
    m_pVSE = new CImageSelRectEx::Vertex;
    m_pVSS2 = new CImageSelRectEx::Vertex;
    m_pVSE2 = new CImageSelRectEx::Vertex;
    m_pVSSb = new CImageSelRectEx::Vertex;
    m_pVSEb = new CImageSelRectEx::Vertex;
    m_pVSS2b = new CImageSelRectEx::Vertex;
    m_pVSE2b = new CImageSelRectEx::Vertex;
}

CImageRectSelCtrl::~CImageRectSelCtrl()
{
}


BEGIN_MESSAGE_MAP(CImageRectSelCtrl, CStatic)
    //{{AFX_MSG_MAP(CImageRectSelCtrl)
    ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageRectSelCtrl message handlers

void CImageRectSelCtrl::onPaint() 
{
    CPaintDC dc(this); // device context for painting
    
    if (m_image.isValid())
    {
        CRgn        rgn;
        CRect        rc;

        getClientRect(&rc);
        rgn.m_hObject = nullptr;
        rgn.CreateRectRgnIndirect(&rc);
        dc.SelectClipRgn(&rgn);

        HDC        hdcTemp = CreateCompatibleDC(dc.m_hDC);
        HBITMAP    hBmpTemp = (HBITMAP)SelectObject(hdcTemp, m_image.getHandle());
        if (m_nScale == 1)
            BitBlt(dc.m_hDC, 0, 0, m_image.m_cx, m_image.m_cy, hdcTemp, 0, 0, SRCCOPY);
        else
            stretchBlt(dc.m_hDC, 0, 0, m_image.m_cx * m_nScale, m_image.m_cy * m_nScale, hdcTemp, 0, 0, m_image.m_cx, m_image.m_cy, SRCCOPY);
        ::SelectObject(hdcTemp, hBmpTemp);
        DeleteDC(hdcTemp);

        dc.SelectClipRgn(nullptr);
    }

    if (m_imgRectSelType != IRST_NONE)
        m_imageSelGrid.drawXor(dc.m_hDC);
}

void CImageRectSelCtrl::initAsRectImg(CRect &rcObj)
{
    m_imgRectSelType = IRST_IMG;

    m_pV4->x = m_pV1->x = rcObj.left;
    m_pV2->y = m_pV1->y = rcObj.top;

    m_pV2->x = m_pV3->x = rcObj.right;
    m_pV3->y = m_pV4->y = rcObj.bottom;

    m_imageSelGrid.addVertexOrderByX(m_pV1);
    m_imageSelGrid.addVertexOrderByX(m_pV4);
    m_imageSelGrid.addVertexOrderByX(m_pV2);
    m_imageSelGrid.addVertexOrderByX(m_pV3);

    m_imageSelGrid.addLine(m_pV1, m_pV2);
    m_imageSelGrid.addLine(m_pV2, m_pV3);
    m_imageSelGrid.addLine(m_pV3, m_pV4);
    m_imageSelGrid.addLine(m_pV4, m_pV1);

    m_imageSelGrid.m_hWnd = m_hWnd;
    m_imageSelGrid.m_nScale = m_nScale;

    CImageSelRectEx::Group        gr;
    gr.bVert = true;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV2);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pV3);
    gr.vVertex.push_back(m_pV4);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pV3);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV4);
    m_imageSelGrid.addGroup(gr);

    m_imageSelGrid.setSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::initAsHorzStretchImg(CRect &rcObj, int nStretchStartX, int nStretchEndX)
{
    m_imgRectSelType = IRST_HORZ_STRETCH_IMG;

    m_pV4->x = m_pV1->x = rcObj.left;
    m_pV2->y = m_pV1->y = rcObj.top;

    m_pV2->x = m_pV3->x = rcObj.right;
    m_pV3->y = m_pV4->y = rcObj.bottom;

    m_pVSSb->x = m_pVSS->x = nStretchStartX;
    m_pVSEb->x = m_pVSE->x = nStretchEndX;
    m_pVSE->y = m_pVSS->y = rcObj.top;
    m_pVSSb->y = m_pVSEb->y = rcObj.bottom;
    m_pVSS->bCanMoveVert = m_pVSSb->bCanMoveVert = m_pVSE->bCanMoveVert = m_pVSEb->bCanMoveVert = false;

    m_imageSelGrid.addVertexOrderByX(m_pV1);
    m_imageSelGrid.addVertexOrderByX(m_pV4);

    m_imageSelGrid.addVertexOrderByX(m_pVSS);
    m_imageSelGrid.addVertexOrderByX(m_pVSSb);

    m_imageSelGrid.addVertexOrderByX(m_pVSE);
    m_imageSelGrid.addVertexOrderByX(m_pVSEb);

    m_imageSelGrid.addVertexOrderByX(m_pV2);
    m_imageSelGrid.addVertexOrderByX(m_pV3);

    m_imageSelGrid.addLine(m_pV1, m_pV2);
    m_imageSelGrid.addLine(m_pV2, m_pV3);
    m_imageSelGrid.addLine(m_pV3, m_pV4);
    m_imageSelGrid.addLine(m_pV4, m_pV1);
    m_imageSelGrid.addLine(m_pVSS, m_pVSSb);
    m_imageSelGrid.addLine(m_pVSE, m_pVSEb);

    m_imageSelGrid.m_hWnd = m_hWnd;
    m_imageSelGrid.m_nScale = m_nScale;

    CImageSelRectEx::Group        gr;

    // top lines
    gr.bVert = true;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSE);
    m_imageSelGrid.addGroup(gr);

    // bottom lines
    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pV3);
    gr.vVertex.push_back(m_pV4);
    gr.vVertex.push_back(m_pVSSb);
    gr.vVertex.push_back(m_pVSEb);
    m_imageSelGrid.addGroup(gr);

    // left line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pV3);
    m_imageSelGrid.addGroup(gr);

    // right line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV4);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSSb);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSE);
    gr.vVertex.push_back(m_pVSEb);
    m_imageSelGrid.addGroup(gr);

    m_imageSelGrid.setSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::initAsVertStretchImg(CRect &rcObj, int nStretchStartY, int nStretchEndY)
{
    m_imgRectSelType = IRST_VERT_STRETCH_IMG;

    m_pV4->x = m_pV1->x = rcObj.left;
    m_pV2->y = m_pV1->y = rcObj.top;

    m_pV2->x = m_pV3->x = rcObj.right;
    m_pV3->y = m_pV4->y = rcObj.bottom;

    m_pVSSb->y = m_pVSS->y = nStretchStartY;
    m_pVSEb->y = m_pVSE->y = nStretchEndY;
    m_pVSE->x = m_pVSS->x = rcObj.left;
    m_pVSSb->x = m_pVSEb->x = rcObj.right;
    m_pVSS->bCanMoveHorz = m_pVSSb->bCanMoveHorz = m_pVSE->bCanMoveHorz = m_pVSEb->bCanMoveHorz = false;

    m_imageSelGrid.addVertexOrderByX(m_pV1);
    m_imageSelGrid.addVertexOrderByX(m_pV4);

    m_imageSelGrid.addVertexOrderByX(m_pVSSb);

    m_imageSelGrid.addVertexOrderByX(m_pVSEb);

    m_imageSelGrid.addVertexOrderByX(m_pVSS);
    m_imageSelGrid.addVertexOrderByX(m_pVSE);
    m_imageSelGrid.addVertexOrderByX(m_pV2);
    m_imageSelGrid.addVertexOrderByX(m_pV3);

    m_imageSelGrid.addLine(m_pV1, m_pV2);
    m_imageSelGrid.addLine(m_pV2, m_pV3);
    m_imageSelGrid.addLine(m_pV3, m_pV4);
    m_imageSelGrid.addLine(m_pV4, m_pV1);
    m_imageSelGrid.addLine(m_pVSS, m_pVSSb);
    m_imageSelGrid.addLine(m_pVSE, m_pVSEb);

    m_imageSelGrid.m_hWnd = m_hWnd;
    m_imageSelGrid.m_nScale = m_nScale;

    CImageSelRectEx::Group        gr;

    // top lines
    gr.bVert = true;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV2);
    m_imageSelGrid.addGroup(gr);

    // bottom lines
    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pV3);
    gr.vVertex.push_back(m_pV4);
    m_imageSelGrid.addGroup(gr);

    // left line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pV3);
    gr.vVertex.push_back(m_pVSSb);
    gr.vVertex.push_back(m_pVSEb);
    m_imageSelGrid.addGroup(gr);

    // right line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV4);
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSE);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSSb);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pVSE);
    gr.vVertex.push_back(m_pVSEb);
    m_imageSelGrid.addGroup(gr);

    m_imageSelGrid.setSelAllRect(m_pV1, m_pV3);
}

void CImageRectSelCtrl::initAsCaptionImg(CRect &rcObj, int nStretchStartX, int nStretchEndX, int nStretchStartX2, int nStretchEndX2)
{
    m_imgRectSelType = IRST_CAPTION_IMG;

    m_pV4->x = m_pV1->x = rcObj.left;
    m_pV2->y = m_pV1->y = rcObj.top;

    m_pV2->x = m_pV3->x = rcObj.right;
    m_pV3->y = m_pV4->y = rcObj.bottom;

    m_pVSSb->x = m_pVSS->x = nStretchStartX;
    m_pVSEb->x = m_pVSE->x = nStretchEndX;

    m_pVSS2b->x = m_pVSS2->x = nStretchStartX2;
    m_pVSE2b->x = m_pVSE2->x = nStretchEndX2;

    m_pVSS2->y = m_pVSE2->y = m_pVSE->y = m_pVSS->y = rcObj.top;
    m_pVSS2b->y = m_pVSE2b->y = m_pVSSb->y = m_pVSEb->y = rcObj.bottom;

    m_pVSS->bCanMoveVert = m_pVSSb->bCanMoveVert = m_pVSE->bCanMoveVert = m_pVSEb->bCanMoveVert = 
        m_pVSS2->bCanMoveVert = m_pVSS2b->bCanMoveVert = m_pVSE2->bCanMoveVert = m_pVSE2b->bCanMoveVert = false;

    m_imageSelGrid.addVertexOrderByX(m_pV1);
    m_imageSelGrid.addVertexOrderByX(m_pV4);

    m_imageSelGrid.addVertexOrderByX(m_pVSS);
    m_imageSelGrid.addVertexOrderByX(m_pVSSb);

    m_imageSelGrid.addVertexOrderByX(m_pVSE);
    m_imageSelGrid.addVertexOrderByX(m_pVSEb);

    m_imageSelGrid.addVertexOrderByX(m_pVSS2);
    m_imageSelGrid.addVertexOrderByX(m_pVSS2b);

    m_imageSelGrid.addVertexOrderByX(m_pVSE2);
    m_imageSelGrid.addVertexOrderByX(m_pVSE2b);

    m_imageSelGrid.addVertexOrderByX(m_pV2);
    m_imageSelGrid.addVertexOrderByX(m_pV3);

    m_imageSelGrid.addLine(m_pV1, m_pV2);
    m_imageSelGrid.addLine(m_pV2, m_pV3);
    m_imageSelGrid.addLine(m_pV3, m_pV4);
    m_imageSelGrid.addLine(m_pV4, m_pV1);
    m_imageSelGrid.addLine(m_pVSS, m_pVSSb);
    m_imageSelGrid.addLine(m_pVSE, m_pVSEb);
    m_imageSelGrid.addLine(m_pVSS2, m_pVSS2b);
    m_imageSelGrid.addLine(m_pVSE2, m_pVSE2b);

    m_imageSelGrid.m_hWnd = m_hWnd;
    m_imageSelGrid.m_nScale = m_nScale;

    CImageSelRectEx::Group        gr;

    // top lines
    gr.bVert = true;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSE);
    gr.vVertex.push_back(m_pVSS2);
    gr.vVertex.push_back(m_pVSE2);
    m_imageSelGrid.addGroup(gr);

    // bottom lines
    gr.vVertex.clear();
    gr.bVert = true;
    gr.vVertex.push_back(m_pV3);
    gr.vVertex.push_back(m_pV4);
    gr.vVertex.push_back(m_pVSSb);
    gr.vVertex.push_back(m_pVSEb);
    gr.vVertex.push_back(m_pVSS2b);
    gr.vVertex.push_back(m_pVSE2b);
    m_imageSelGrid.addGroup(gr);

    // left line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV2);
    gr.vVertex.push_back(m_pV3);
    m_imageSelGrid.addGroup(gr);

    // right line
    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pV1);
    gr.vVertex.push_back(m_pV4);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSS);
    gr.vVertex.push_back(m_pVSSb);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSE);
    gr.vVertex.push_back(m_pVSEb);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSS2);
    gr.vVertex.push_back(m_pVSS2b);
    m_imageSelGrid.addGroup(gr);

    gr.vVertex.clear();
    gr.bVert = false;
    gr.vVertex.push_back(m_pVSE2);
    gr.vVertex.push_back(m_pVSE2b);
    m_imageSelGrid.addGroup(gr);

    m_imageSelGrid.setSelAllRect(m_pV1, m_pV3);
}


void CImageRectSelCtrl::getRect(CRect &rcObj)
{
    rcObj.left = m_pV1->x;
    rcObj.top = m_pV1->y;
    rcObj.right = m_pV3->x;
    rcObj.bottom = m_pV3->y;
}

int CImageRectSelCtrl::getStretchStart()
{
    if (m_imgRectSelType == IRST_VERT_STRETCH_IMG)
        return m_pVSS->y;
    else
        return m_pVSS->x;
}

int CImageRectSelCtrl::getStretchEnd()
{
    if (m_imgRectSelType == IRST_VERT_STRETCH_IMG)
        return m_pVSE->y;
    else
        return m_pVSE->x;
}

int CImageRectSelCtrl::getStretchStart2()
{
    return m_pVSS2->x;
}

int CImageRectSelCtrl::getStretchEnd2()
{
    return m_pVSE2->x;
}

void CImageRectSelCtrl::setRectX(int x)
{
    int        nOffset = x - m_pV1->x;
    m_imageSelGrid.offsetAllVertex(nOffset, 0);
}

void CImageRectSelCtrl::setRectY(int y)
{
    int        nOffset = y - m_pV1->y;
    m_imageSelGrid.offsetAllVertex(0, nOffset);
}

void CImageRectSelCtrl::setRectCx(int cx)
{
    int        x = m_pV1->x + cx;
    m_imageSelGrid.setVertex(m_pV2, x, 0);
}

void CImageRectSelCtrl::setRectCy(int cy)
{
    int        y = m_pV1->y + cy;
    m_imageSelGrid.setVertex(m_pV3, 0, y);
}

void CImageRectSelCtrl::setStretchStart(int nStretchStart)
{
    m_pVSS->x = nStretchStart;
    m_pVSSb->x = nStretchStart;
}

void CImageRectSelCtrl::setStretchEnd(int nStretchEnd)
{
    m_pVSE->x = nStretchEnd;
    m_pVSEb->x = nStretchEnd;
}

void CImageRectSelCtrl::setStretchStart2(int nStretchStart2)
{
    m_pVSS2->x = nStretchStart2;
    m_pVSS2b->x = nStretchStart2;
}

void CImageRectSelCtrl::setStretchEnd2(int nStretchEnd2)
{
    m_pVSE2->x = nStretchEnd2;
    m_pVSE2b->x = nStretchEnd2;
}

int CImageRectSelCtrl::onCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CStatic::onCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CImageRectSelCtrl::onDestroy() 
{
    CStatic::onDestroy();
    
    m_image.destroy();
    m_imageSelGrid.close();
}

void CImageRectSelCtrl::setScale(int nScale)
{
    m_imageSelGrid.m_nScale = nScale;
    m_nScale = nScale;
}

bool CImageRectSelCtrl::setImageFile(cstr_t szImage)
{
    CSFImage        image;

    image.loadFromSRM(&g_skinFactory, szImage);

    m_image.destroy();
    m_image.attach(RawImageToHBitmap(image));

    if (m_hWnd)
        invalidate();

    return true;
}

void CImageRectSelCtrl::onLButtonDown(uint32_t nFlags, CPoint point) 
{
    setFocus();

    if (m_imgRectSelType == IRST_NONE)
        return;

    point.x /= m_nScale;
    point.y /= m_nScale;

    m_imageSelGrid.onLButtonDown(nFlags, point);

    setCapture();

    CStatic::onLButtonDown(nFlags, point);
}

void CImageRectSelCtrl::onLButtonUp(uint32_t nFlags, CPoint point) 
{
    point.x /= m_nScale;
    point.y /= m_nScale;

    m_imageSelGrid.onLButtonUp(nFlags, point);

    releaseCapture();
}

void CImageRectSelCtrl::onMouseMove(uint32_t nFlags, CPoint point) 
{
    point.x /= m_nScale;
    point.y /= m_nScale;

    CString        str;
    str.Format("X: %4d, Y: %4d", point.x, point.y);
    ::setDlgItemText(::getParent(m_hWnd), IDC_S_CURSOR, str);

    m_imageSelGrid.onMouseMove(nFlags, point);
}

LRESULT CImageRectSelCtrl::windowProc(uint32_t message, WPARAM wParam, LPARAM lParam) 
{
    if (message == WM_GETDLGCODE)
    {
        return DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_DEFPUSHBUTTON;
    }
    
    return CStatic::windowProc(message, wParam, lParam);
}

void CImageRectSelCtrl::onKeyDown(uint32_t nChar, uint32_t nRepCnt, uint32_t nFlags) 
{
    if (m_imgRectSelType != IRST_NONE)
        m_imageSelGrid.onKeyDown(nChar, nRepCnt, nFlags);

    CStatic::onKeyDown(nChar, nRepCnt, nFlags);
}

bool CImageRectSelCtrl::OnEraseBkgnd(CDC* pDC) 
{
    CRect    rc;
    getClientRect(&rc);
    pDC->FillSolidRect(&rc, RGB(255, 255, 255));

    return true;
}
