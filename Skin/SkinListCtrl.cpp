// SkinListCtrl.cpp: implementation of the CSkinListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinListCtrl.h"
#include "SkinFrameCtrl.h"


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinListCtrl, "ListCtrl")

CSkinListCtrl::CSkinListCtrl()
{
    setDataSource(this);
}


CSkinListCtrl::~CSkinListCtrl()
{
    for (int i = 0; i < (int)m_vRows.size(); i++)
    {
        Row *row = m_vRows[i];
        delete row;
    }
}

int CSkinListCtrl::getRowCount() const
{
    return (int)m_vRows.size();
}

bool CSkinListCtrl::isRowSelected(int nRow) const
{
    if (nRow >= 0 && nRow <= (int)m_vRows.size())
    {
        Row    *row = m_vRows[nRow];
        return row->isSelected();
    }

    return false;
}


void CSkinListCtrl::addColumn(cstr_t szCol, int nWidth, int colType, bool bClickable)
{
    CSkinListView::addColumn(szCol, nWidth, colType, bClickable);

    int col = getColumnCount() - 1;

    for (uint32_t i = 0; i < m_vRows.size(); i++)
    {
        Row *row = m_vRows[i];
        row->vItems.push_back(newItem(col));
    }
}

int CSkinListCtrl::getItemText(int nItem, int nSubItem, char * lpszText, int nLen)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    cstr_t szText = getCellText(nItem, nSubItem);
    if (szText != nullptr)
    {
        strcpy_safe(lpszText, nLen, szText);
        return (int)strlen(szText);
    }

    lpszText[0] = '\0';

    return -1;
}


int CSkinListCtrl::getItemText(int nItem, int nSubItem, string &strText)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    cstr_t szText = getCellText(nItem, nSubItem);
    if (szText != nullptr)
    {
        strText = szText;
        return strText.size();
    }

    strText = "";

    return -1;
}


bool CSkinListCtrl::setItemText(int nItem, int nSubItem, cstr_t lpszText, bool bRedraw)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row *row = m_vRows[nItem];
        if (m_vHeading[nSubItem]->colType == CColHeader::TYPE_TEXT)
        {
            ItemString *item = (ItemString *)row->vItems[nSubItem];
            item->text = lpszText;
            if (bRedraw)
                invalidateItem(nItem);
            return true;
        }
    }

    return false;
}

bool CSkinListCtrl::setItemImage(int nItem, int nSubItem, RawImageData *image, bool bRedraw)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        if (m_vHeading[nSubItem]->colType == CColHeader::TYPE_IMAGE)
        {
            ItemImage *item = (ItemImage *)row->vItems[nSubItem];
            item->image->attach(image);
            if (bRedraw)
                invalidateItem(nItem);
            return true;
        }
    }

    return false;
}

int CSkinListCtrl::insertItem(int nItem, cstr_t lpszItem, int nImageIndex, uint32_t nItemData, bool bRedraw)
{
    assert(m_vHeading.size() > 0);
    if (m_vHeading.size() == 0)
        return -1;

    Row    *row = new Row(nImageIndex, nItemData);

    // alloc space for the new row
    for (int i = 0; i < (int)m_vHeading.size(); i++)
    {
        row->vItems.push_back(newItem(i));
    }

    if (m_vHeading[0]->colType == CColHeader::TYPE_TEXT)
    {
        ItemString *itemStr = (ItemString *)row->vItems[0];
        itemStr->text = lpszItem;
    }

    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        m_vRows.insert(m_vRows.begin() + nItem, row);
    }
    else
    {
        m_vRows.push_back(row);
        nItem = m_vRows.size() - 1;
    }

    setScrollInfo(bRedraw);

    if (bRedraw && nItem <= getLastVisibleRow())
        invalidate();

    return nItem;
}


bool CSkinListCtrl::deleteItem(int nItem, bool bRedraw)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        m_vRows.erase(m_vRows.begin() + nItem);
        delete row;

        if (bRedraw)
            invalidate();

        return true;
    }
    else
        return false;
}


bool CSkinListCtrl::deleteSelectedItems(bool bRedraw)
{
    for (int i = (int)m_vRows.size() - 1; i >= 0; i--)
    {
        Row        *row = m_vRows[i];
        if (row->isSelected())
        {
            delete row;
            m_vRows.erase(m_vRows.begin() + i);
        }
    }

    if (bRedraw)
        invalidate();

    return true;
}


bool CSkinListCtrl::deleteAllItems(bool bRedraw)
{
    for (int i = 0; i < (int)m_vRows.size(); i++)
    {
        Row        *row = m_vRows[i];
        delete row;
    }
    m_vRows.clear();

    m_nEndSelRow = -1;
    m_nBegSelRow = -1;

    if (bRedraw)
        invalidate();

    if (m_pVertScrollBar)
    {
        m_pVertScrollBar->setScrollPos(0);
        m_pVertScrollBar->disableScrollBar();
    }

//     if (m_pHorzScrollBar)
//     {
//         m_pHorzScrollBar->setScrollPos(0);
//         m_pHorzScrollBar->disableScrollBar();
//     }

    return true;
}

void CSkinListCtrl::setRowSelectionState(int nIndex, bool bSelected)
{
    assert(nIndex >= 0 && nIndex < (int)m_vRows.size());
    if (nIndex < 0 || nIndex >= (int)m_vRows.size())
        return;

    Row        *row = m_vRows[nIndex];
    row->setSelected(bSelected);
}

int CSkinListCtrl::getItemImageIndex(int row)
{
    assert(row >= 0 && row < m_vRows.size());
    if (row < 0 || row >= (int)m_vRows.size())
        return -1;

    return m_vRows[row]->nImageIndex;
}

bool CSkinListCtrl::setItemImageIndex(int nItem, int nImageIndex, bool bRedraw)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        row->nImageIndex = nImageIndex;
        if (bRedraw)
            invalidateItem(nItem);
        return true;
    }

    return false;
}

uint32_t CSkinListCtrl::getItemData(int nItem)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        return row->nItemData;
    }

    return -1;
}

void CSkinListCtrl::setItemData(int nItem, uint32_t nItemData)
{
    assert(nItem >= 0 && nItem < (int)m_vRows.size());
    if (nItem >= 0 && nItem < (int)m_vRows.size())
    {
        Row        *row = m_vRows[nItem];
        row->nItemData = nItemData;
    }
}

cstr_t CSkinListCtrl::getCellText(int row, int col)
{
    assert(row >= 0 && row < m_vRows.size());
    if (row < 0 || row >= (int)m_vRows.size())
        return "";

    Row *pRow = m_vRows[row];
    if (col < 0 || col >= (int)pRow->vItems.size())
        return "";

    ItemString *item = (ItemString*) pRow->vItems[col];
    return item->text.c_str();
}

CRawImage *CSkinListCtrl::getCellImage(int row, int col)
{
    assert(row >= 0 && row < m_vRows.size());
    if (row < 0 || row >= (int)m_vRows.size())
        return nullptr;

    Row *pRow = m_vRows[row];
    if (col < 0 || col >= (int)pRow->vItems.size())
        return nullptr;

    ItemImage *item = (ItemImage*) pRow->vItems[col];
    return item->image;
}

CSkinListCtrl::Item *CSkinListCtrl::newItem(int nCol)
{
    assert(nCol >= 0 && nCol < (int)m_vHeading.size());
    int type = m_vHeading[nCol]->colType;
    if (type == CColHeader::TYPE_TEXT)
        return new ItemString();
    else if (type == CColHeader::TYPE_IMAGE)
        return new ItemImage();
    else
    {
        assert(0 && "Undefined column type.");
        return nullptr;
    }
}

bool CSkinListCtrl::canOffsetSelectedRow(bool bDown)
{
    if (m_vRows.size() == 0)
        return false;

    if (getNextSelectedItem() == -1)
        return false;

    if (bDown)
    {
        if (m_vRows.back()->isSelected())
            return false;
    }
    else
    {
        // move up
        if (m_vRows.front()->isSelected())
            return false;
    }

    return true;
}


bool CSkinListCtrl::offsetAllSelectedRow(bool bDown)
{
    if (m_vRows.size() == 0)
        return false;

    if (bDown)
    {
        if (m_vRows.back()->isSelected())
            return false;

        for (int i = (int)m_vRows.size() - 1; i >= 0; i--)
        {
            Row        *row = m_vRows[i];
            if (row->isSelected())
            {
                Row    *rowTemp;
                rowTemp = m_vRows[i + 1];
                m_vRows[i + 1] = m_vRows[i];
                m_vRows[i] = rowTemp;
            }
        }
    }
    else
    {
        // move up
        if (m_vRows.front()->isSelected())
            return false;

        for (int i = 0; i < (int)m_vRows.size(); i++)
        {
            Row        *row = m_vRows[i];
            if (row->isSelected())
            {
                Row    *rowTemp;
                rowTemp = m_vRows[i - 1];
                m_vRows[i - 1] = m_vRows[i];
                m_vRows[i] = rowTemp;
            }
        }
    }

    invalidate();
    return true;
}
