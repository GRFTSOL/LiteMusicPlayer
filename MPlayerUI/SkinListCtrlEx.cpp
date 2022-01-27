#include "Skin.h"
#include "Helper.h"
#include "SkinListCtrlEx.h"


UIOBJECT_CLASS_NAME_IMP(CSkinListCtrlEx, "ListCtrlEx")

CSkinListCtrl::Item *CSkinListCtrlEx::newItem(int col)
{
    int colType = m_vHeading[col]->colType;
    if (colType == TYPE_TEXT_EX)
        return new ItemStringEx();
    else
        return CSkinListCtrl::newItem(col);
}

void CSkinListCtrlEx::addColumn(cstr_t szCol, int nWidth)
{
    CSkinListCtrl::addColumn(szCol, nWidth, TYPE_TEXT_EX);
}

bool CSkinListCtrlEx::setItemText(int nItem, int nSubItem, cstr_t lpszText, bool bRedraw)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        ItemStringEx    *item = (ItemStringEx*)row->getSubItem(nSubItem);
        if (item)
        {
            item->text = lpszText;
            item->vTextColor.clear();
            if (bRedraw)
                invalidateItem(nItem);
            return true;
        }
    }

    return false;
}


int CSkinListCtrlEx::insertItem(int nItem, cstr_t lpszItem, int nImageIndex, uint32_t nItemData, bool bRedraw)
{
    ItemStringEx::VecTextColor    vTextColor;

    return insertItem(nItem, lpszItem, vTextColor, nImageIndex, nItemData, bRedraw);
}


bool CSkinListCtrlEx::setItemTextEx(int nItem, int nSubItem, cstr_t lpszText, const ItemStringEx::VecTextColor & vTextColor)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        ItemStringEx        *item = (ItemStringEx*)row->getSubItem(nSubItem);
        if (item)
        {
            item->text = lpszText;
            item->vTextColor = vTextColor;
            return true;
        }
    }

    return false;
}


int CSkinListCtrlEx::insertItem(int nItem, cstr_t lpszItem, const ItemStringEx::VecTextColor & vTextColor, int nImageIndex, uint32_t nItemData, bool bRedraw)
{
    int    n = CSkinListCtrl::insertItem(nItem, lpszItem, nImageIndex, nItemData, false);

    if (n >= 0 && n < (int)m_vRows.size())
    {
        ItemStringEx        *item = (ItemStringEx *)(m_vRows[n]->vItems.front());
        item->vTextColor = vTextColor;
    }

    if (bRedraw)
    {
        setScrollInfo(true);

        if (nItem >= m_nFirstVisibleRow && nItem <= m_nFirstVisibleRow + m_rcObj.height() / m_nLineHeight)
            invalidate();
    }

    return nItem;
}

void CSkinListCtrlEx::drawCell(int row, int col, CRect &rcCell, CRawGraph *canvas, CColor &clrText)
{
    int colType = m_vHeading[col]->colType;
    if (colType == TYPE_TEXT_EX)
    {
        Row *pRow = m_vRows[row];
        return drawCellTextEx((ItemStringEx*) pRow->vItems[col], rcCell, canvas, clrText);
    }
    else
        return CSkinListCtrl::drawCell(row, col, rcCell, canvas, clrText);
}

void CSkinListCtrlEx::drawCellTextEx(ItemStringEx *item, CRect &rcItem, CRawGraph *canvas, CColor &clrText)
{
    string::size_type            nPos = 0, nLen, i = 0;
    CRect        rc = rcItem;

    while (nPos < item->text.size())
    {
        CColor clr;
        if (i >= item->vTextColor.size())
        {
            nLen = item->text.size() - nPos;
            clr = clrText;
        }
        else
        {
            nLen = item->vTextColor[i].nCountOfText;
            if (nPos + nLen > item->text.size())
                nLen = item->text.size() - nPos;
            clr = getColor(item->vTextColor[i].nClrIndex);
        }

        canvas->setTextColor(clr);

        if (m_font.isOutlined())
        {
            canvas->drawTextOutlined(item->text.c_str() + nPos, nLen, rc, 
                clr, 
                m_font.getColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
        }
        else
            canvas->drawText(item->text.c_str() + nPos, nLen, rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);

        CSize size;
        canvas->getTextExtentPoint32(item->text.c_str() + nPos, nLen, &size);

        nPos += nLen;
        rc.left += size.cx;
        i++;
    }
}

