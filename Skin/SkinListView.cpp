#include "SkinTypes.h"
#include "Skin.h"
#include "SkinListView.h"
#include "SkinFrameCtrl.h"


//////////////////////////////////////////////////////////////////////////

#define SZ_CUSTOMIZED_CLR   "CustomizedColor"

UIOBJECT_CLASS_NAME_IMP(CSkinListView, "ListView")

CSkinListView::CSkinListView() {
    m_bHorzScrollBar = m_bVertScrollBar = true;
    m_bEnableBorder = true;

    m_vColors.resize(CN_CUSTOMIZED_START);

    m_nXMargin = 2;
    m_bDrawHeader = false;
    m_bEnableSort = false;

    m_msgNeed = UO_MSG_WANT_RBUTTON | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

    // setBkColor(CColor(GetSysColor(COLOR_WINDOW)));
    setBkColor(CColor(RGB(192, 192, 192)));
    setSelRowBkColor(CColor(RGB(128, 128, 128)));
    m_vColors[CN_SEL_TEXT].set(RGB(255, 255, 255));

    m_nLineHeightOrg = -1;
    m_nLineHeight = 16;

    m_imageListIconCx = 16;
    m_imageListRightSpace = 4;

    m_bDrawBorder = false;
    m_bDrawHeader = false;
    m_nHeaderHeight = 16;
    CColor clrPen(RGB(128, 128, 128));
    m_penLine.createSolidPen(1, clrPen);
    m_nSortedCol = 0;
    m_bAscending = false;
    m_bAdjustColWidth = false;

    m_nBorderWidth = 0;

    m_bHoverMouseSel = false;
    m_bClickBtDown = false;

    m_nFirstVisibleRow = 0;
    m_nBegSelRow = -1;
    m_nEndSelRow = -1;

    m_dataSource = nullptr;
}

CSkinListView::~CSkinListView() {
}

void CSkinListView::setDataSource(IListViewDataSource *pDataSource) {
    m_dataSource = pDataSource;
    if (pDataSource != nullptr) {
        pDataSource->setView(this);
    }
}

void CSkinListView::notifyDataChanged() {
    setScrollInfo();
    invalidate();
}

void CSkinListView::loadColumnWidth(cstr_t szProperty) {
    VecStrings vWidth;
    strSplit(g_profile.getString(szProperty, ""), ',', vWidth);
    for (uint32_t i = 0; i < vWidth.size() && i < m_vHeading.size(); i++) {
        if (vWidth[i].empty()) {
            continue;
        }
        setColumnWidth(i, atoi(vWidth[i].c_str()));
    }
}

void CSkinListView::saveColumnWidth(cstr_t szProperty) {
    VecInts vWidth;
    for (int i = 0; i < getColumnCount(); i++) {
        vWidth.push_back(getColumnWidth(i));
    }
    string str = strJoin(vWidth.begin(), vWidth.end(), "%d", ",");
    g_profile.writeString(szProperty, str.c_str());
}

void CSkinListView::onCreate() {
    m_font.onCreate(m_pSkin);

    CSkinScrollFrameCtrlBase::onCreate();

    m_nLineHeight = m_font.getHeight() + 4;
    if (m_nLineHeight < m_nLineHeightOrg) {
        m_nLineHeight = m_nLineHeightOrg;
    }

    // if in Editor mode, don't create scrollbar
    if (m_pSkin->isInEditorMode()) {
        return;
    }

    string curFile;
    if (m_pSkin->getSkinFactory()->getResourceMgr()->getResourcePathName("list_ctrl_resize_col.cur", curFile)) {
        m_curResizeCol.loadCursorFromFile(curFile.c_str());
    }
}

bool CSkinListView::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (isPropertyName(szProperty, "BgColor")) {
        VecStrings vColor;
        strSplit(szValue, ',', vColor);
        trimStr(vColor);
        if (vColor.size() > 0) {
            getColorValue(getColor(CN_BG), vColor[0].c_str());
        }
        if (vColor.size() > 1) {
            getColorValue(getColor(CN_ALTER_BG), vColor[1].c_str());
        } else {
            getColor(CN_ALTER_BG) = getColor(CN_BG);
        }

        m_clrBg = getColor(CN_BG);
        m_bgType = BG_COLOR;
        return true;
    }

    if (CSkinScrollFrameCtrlBase::setProperty(szProperty, szValue)) {
        return true;
    }

    if (strcasecmp(szProperty, "SelBgColor") == 0) {
        getColorValue(getColor(CN_SEL_BG), szValue);
    } else if (strcasecmp(szProperty, "SelTextColor") == 0) {
        getColorValue(getColor(CN_SEL_TEXT), szValue);
    } else if (*szProperty == 'C' && strncasecmp(szProperty, SZ_CUSTOMIZED_CLR, strlen(SZ_CUSTOMIZED_CLR)) == 0) {
        CColor clr;
        int nColorName = CN_CUSTOMIZED_START + atoi(szValue + strlen(SZ_CUSTOMIZED_CLR));

        getColorValue(clr, szValue);
        setColor(nColorName, clr);
    } else if (m_font.setProperty(szProperty, szValue)) {
        return true;
    } else if (strcasecmp(szProperty, "ImageList") == 0) {
        m_strImageList = szValue;
        m_imageList.destroy();
        m_imageList.load(m_pSkin->getSkinFactory(), szValue,
            m_imageListIconCx);
    } else if (strcasecmp(szProperty, "ImageListIconCx") == 0) {
        m_imageListIconCx = atoi(szValue);
        m_imageList.setIconCx(m_imageListIconCx);
    } else if (isPropertyName(szProperty, "drawHeader")) {
        m_bDrawHeader = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "DrawBorder")) {
        m_bDrawBorder = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "EnableSort")) {
        m_bEnableSort = isTRUE(szValue);
    } else if (isPropertyName(szProperty, "HeaderHeight")) {
        m_nHeaderHeight = atoi(szValue);
    } else if (isPropertyName(szProperty, "LineHeight")) {
        m_nLineHeightOrg = atoi(szValue);
        if (m_nLineHeight < m_nLineHeightOrg) {
            m_nLineHeight = m_nLineHeightOrg;
        }
    } else if (isPropertyName(szProperty, "XMargin")) {
        m_nXMargin = atoi(szValue);
    } else if (isPropertyName(szProperty, "LineColor")) {
        CColor clrPen(RGB(128, 128, 128));
        getColorValue(clrPen, szValue);
        m_penLine.createSolidPen(1, clrPen);
    } else if (isPropertyName(szProperty, "ImageSortedHeader")) {
        m_imageSortedHeader.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (isPropertyName(szProperty, "ImageHeader")) {
        m_imageHeader.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (isPropertyName(szProperty, "ImageSortFlag")) {
        m_imageAscending.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
        m_imageAscending.m_cx /= 2;
        m_imageDescending.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
        m_imageDescending.m_x = m_imageDescending.m_cx / 2;
        m_imageDescending.m_cx /= 2;
    } else if (isPropertyName(szProperty, "ImageBorder")) {
        m_imageBorder.loadFromSRM(m_pSkin->getSkinFactory(), szValue);
    } else if (isPropertyName(szProperty, "BorderWidth")) {
        m_nBorderWidth = atoi(szValue);
    } else if (isPropertyName(szProperty, "MouseHoverSel")) {
        m_bHoverMouseSel = isTRUE(szValue);
    } else {
        return false;
    }

    return true;
}

#ifdef _SKIN_EDITOR_
void CSkinListView::enumProperties(CUIObjProperties &listProperties) {
    CSkinScrollFrameCtrlBase::enumProperties(listProperties);

    listProperties.addPropColor("BgColor", getColor(CN_BG));
    listProperties.addPropColor("SelBgColor", getColor(CN_SEL_BG));
    listProperties.addPropColor("SelTextColor", getColor(CN_SEL_TEXT));
    m_font.enumProperties(listProperties);
    listProperties.addPropImageFile("ImageList", m_strImageList.c_str());

    for (vector<CColor>::size_type i = CN_CUSTOMIZED_START; i < m_vColors.size(); i++) {
        listProperties.addPropColor(stringPrintf("%s%d", SZ_CUSTOMIZED_CLR, i - CN_CUSTOMIZED_START).c_str(),
            getColor(i));
    }
}
#endif // _SKIN_EDITOR_

void CSkinListView::onSize() {
    CSkinScrollFrameCtrlBase::onSize();

    setScrollInfo(false);
    setHorzScrollInfo(false);

    if (m_vHeading.size() == 0) {
        return;
    }

    int w = m_rcContent.width() - m_nXMargin * 2;

    if (m_vHeading.size() == 1) {
        m_vHeading[0]->nWidth = w;
    } else {
        int i;
        int n = 0;
        for (i = 0; i < (int)m_vHeading.size(); i++) {
            n += m_vHeading[i]->nWidth;
        }
        if (n < w) {
            m_vHeading[m_vHeading.size() - 1]->nWidth += w - n;
        }
    }

    if (getLinesOfPerPage() >= (int)getRowCount()) {
        m_nFirstVisibleRow = 0;
    }
}

void CSkinListView::onAdjustHue(float hue, float saturation, float luminance) {
    CSkinScrollFrameCtrlBase::onAdjustHue(hue, saturation, luminance);

    for (size_t i = 0; i < m_vColors.size(); i++) {
        m_pSkin->getSkinFactory()->getAdjustedHueResult(m_vColors[i]);
    }

    m_font.onAdjustHue(m_pSkin, hue, saturation, luminance);
}

void CSkinListView::onVScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    if (getRowCount() == 0) {
        return;
    }

    assert(nPos >= 0 && nPos < (int)getRowCount());
    if (nPos >= 0 && nPos < (int)getRowCount()) {
        m_nFirstVisibleRow = nPos;
        invalidate();
    }
}


void CSkinListView::onHScroll(uint32_t nSBCode, int nPos, IScrollBar *pScrollBar) {
    invalidate();
}


void CSkinListView::setBkColor(const CColor &clrBk) {
    getColor(CN_BG) = clrBk;
}


void CSkinListView::setSelRowBkColor(const CColor &clrBk) {
    getColor(CN_SEL_BG) = clrBk;
}


void CSkinListView::setFont(const CFontInfo &font) {
    m_font.create(font);
    m_nLineHeight = m_font.getHeight() + 4;
    if (m_nLineHeight < m_nLineHeightOrg) {
        m_nLineHeight = m_nLineHeightOrg;
    }
}


void CSkinListView::setTextColor(const CColor &clrText) {
    m_font.setProperty("TextColor", colorToStr(clrText).c_str());
}

void CSkinListView::setColor(int nColorName, const CColor &clr) {
    assert(m_vColors.size() >= CN_CUSTOMIZED_START);
    if (nColorName >= (int)m_vColors.size()) {
        if (nColorName > CN_MAX) {
            return;
        }
        m_vColors.resize(nColorName + 1);
    } else if (nColorName == CN_TEXT) {
        m_font.setProperty("TextColor", colorToStr(clr).c_str());
    }

    m_vColors[nColorName] = clr;
}


CColor & CSkinListView::getColor(int nColorName) {
    assert(m_vColors.size() >= CN_CUSTOMIZED_START);
    if (nColorName < 0 || nColorName >= (int)m_vColors.size()) {
        assert(0 && "Invalid color name ID");
        return m_vColors[0];
    } else if (nColorName == CN_TEXT) {
        m_vColors[CN_TEXT] = m_font.getTextColor(m_enable);
    }

    return m_vColors[nColorName];
}

void CSkinListView::addColumn(cstr_t szCol, int nWidth, int colType, bool bClickable) {
    m_vHeading.push_back(new CColHeader(szCol, nWidth, colType, bClickable));

    setHorzScrollInfo(false);
}

int CSkinListView::getColumnWidth(int nCol) const {
    assert(nCol >= 0 && nCol < (int)m_vHeading.size());
    if (nCol >= 0 && nCol < (int)m_vHeading.size()) {
        return m_vHeading[nCol]->nWidth;
    } else {
        return -1;
    }
}


bool CSkinListView::setColumnWidth(int nCol, int cx) {
    assert(nCol >= 0 && nCol < (int)m_vHeading.size());
    if (nCol >= 0 && nCol < (int)m_vHeading.size()) {
        m_vHeading[nCol]->nWidth = cx;
        return true;
    } else {
        return false;
    }
}

void CSkinListView::setLineHeight(int nLineHeight) {
    m_nLineHeightOrg = nLineHeight;
    if (m_nLineHeight < m_nLineHeightOrg) {
        m_nLineHeight = m_nLineHeightOrg;
    }
}

uint32_t CSkinListView::getSelectedCount() const {
    int nSelectedCount = 0;

    for (int i = 0; i < (int)getRowCount(); i++) {
        if (m_dataSource->isRowSelected(i)) {
            nSelectedCount++;
        }
    }
    return nSelectedCount;
}

int CSkinListView::getNextSelectedItem(int nPos) const {
    if (nPos < -1) {
        nPos = -1;
    }
    for (int i = nPos + 1; i < (int)getRowCount(); i++) {
        if (m_dataSource->isRowSelected(i)) {
            return i;
        }
    }

    return -1;
}

void CSkinListView::setItemSelectionState(int nIndex, bool bSelected) {
    assert(nIndex >= 0 && nIndex < (int)getRowCount());
    if (nIndex < 0 || nIndex >= (int)getRowCount()) {
        return;
    }

    m_nBegSelRow = m_nEndSelRow = nIndex;

    CPaintUpdater updater(this);
    CSelChangedCheck selChecher(this);

    m_dataSource->setRowSelectionState(nIndex, bSelected);

    if (selChecher.isSelectChanged()) {
        updater.setUpdateFlag(true);
        sendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
    }
}

void CSkinListView::makeSureRowVisible(int nRow) {
    if (nRow >= m_nFirstVisibleRow && nRow <= getLastVisibleRow()) {
        return;
    }

    if (nRow < m_nFirstVisibleRow) {
        m_nFirstVisibleRow = nRow;
    } else {
        m_nFirstVisibleRow = nRow - (getLinesOfPerPage() - 1);
        if (m_nFirstVisibleRow < 0) {
            m_nFirstVisibleRow = 0;
        }
    }

    if (m_pVertScrollBar) {
        m_pVertScrollBar->setScrollPos(m_nFirstVisibleRow);
    }
}

void CSkinListView::increaseHeight(int value) {
    if (value == 0) {
        return;
    }

    m_formHeight.increase(value);
    m_pContainer->recalculateUIObjSizePos(this);
}

int CSkinListView::getContentHeight() const {
    int nHeight = 0;

    if (m_bDrawHeader) {
        nHeight += m_nHeaderHeight;
    }

    nHeight += m_nLineHeight * getRowCount();
    return nHeight;
}

void CSkinListView::getRowColAtPoint(CPoint pt, int &row, int &col, CPoint *ptAtItemCell, bool *bItemClickable) {
    row = col = -1;
    if (bItemClickable) {
        *bItemClickable = false;
    }

    // Col
    pt.x -= m_rcContent.left + m_nXMargin;

    if (pt.x >= 0 && pt.x <= m_rcContent.width()) {
        if (m_pHorzScrollBar && m_pHorzScrollBar->isEnabled()) {
            pt.x += m_pHorzScrollBar->getScrollPos();
        }
        for (uint32_t i = 0; i < m_vHeading.size(); i++) {
            if (pt.x < m_vHeading[i]->nWidth) {
                if (ptAtItemCell) {
                    ptAtItemCell->x = pt.x;
                }
                col = i;
            }
            pt.x -= m_vHeading[i]->nWidth;
        }
    }

    // Row
    pt.y -= m_rcContent.top;
    if (m_bDrawHeader) {
        pt.y -= m_nHeaderHeight;
    }

    if (pt.y >= 0 && pt.y <= m_rcContent.height()) {
        int n = m_nFirstVisibleRow + pt.y / m_nLineHeight;
        if (ptAtItemCell) {
            ptAtItemCell->y = pt.y % m_nLineHeight;
        }
        if (n < (int)getRowCount()) {
            row = n;
        }
    }

    if (bItemClickable && col != -1 && row != -1 && ptAtItemCell != nullptr
        && m_vHeading[col]->bClickable) {
        if (isClickedOn(row, col, m_vHeading[col], ptAtItemCell->x, ptAtItemCell->y)) {
            *bItemClickable = true;
        }
    }
}

void CSkinListView::setClickCursor() {
    if (!m_curHand.isValid()) {
        m_curHand.loadStdCursor(Cursor::C_HAND);
    }

    setCursor(m_curHand);
}

void CSkinListView::clearAllSelMark() {
    for (int i = 0; i < (int)getRowCount(); i++) {
        if (m_dataSource->isRowSelected(i)) {
            m_dataSource->setRowSelectionState(i, false);
        }
    }
}

int CSkinListView::getLastVisibleRow() const {
    int n;
    n = m_nFirstVisibleRow + getLinesOfPerPage() - 1;
    if (n > (int)getRowCount() - 1) {
        n = (int)getRowCount() - 1;
    }
    return n;
}


void CSkinListView::reverseSelectStatOfRow(bool bClearOldSel, int nRow) {
    assert(nRow >= -1 && nRow < (int)getRowCount());
    if (bClearOldSel) {
        clearAllSelMark();
    }

    if (nRow != -1) {
        m_dataSource->setRowSelectionState(nRow, !m_dataSource->isRowSelected(nRow));
    }
}

void CSkinListView::selectRangeRow(int nBeg, int nEnd) {
    if (nEnd == -1) {
        return;
    }

    assert(nEnd >= 0 && nEnd < (int)getRowCount());
    assert(nBeg >= -1 && nBeg < (int)getRowCount());

    clearAllSelMark();

    if (nBeg == -1) {
        nBeg = 0;
    }

    m_nBegSelRow = nBeg;
    m_nEndSelRow = nEnd;

    if (nBeg > nEnd) {
        int n = nBeg;
        nBeg = nEnd;
        nEnd = n;
    }
    for (; nBeg <= nEnd; nBeg++) {
        m_dataSource->setRowSelectionState(nBeg, true);
    }
}


void CSkinListView::recalculateColWidth(CRawGraph *canvas) {
    /*
    int        i;

    assert(m_vHeading.size() == 2);
    if (m_vHeading.size() != 2)
        return;

    // Should recalculate column width?
    for (i = 0; i < (int)m_vHeading.size(); i++)
    {
        if (m_vHeading[i].nWidth == -1)
            break;
    }
    if (i == m_vHeading.size())
        return;

    CSize size;
    int        nWidthMax = 0;
    for (i = 0; i < (int)getRowCount(); i++)
    {
        int            x = rc.left;
        LC_ROW        *row = m_vRows[i];
        LC_ITEM        *pItem = row->getSubItem(1);

        if (canvas->getTextExtentPoint32(pItem->strText.c_str(), pItem->strText.size(), &size))
        {
            if (nWidthMax < size.cx)
                nWidthMax = size.cx;
        }
    }

    m_vHeading[1].nWidth = nWidthMax;
    m_vHeading[0].nWidth = 
*/
}


void CSkinListView::setScrollInfo(bool bRedraw) {
    int nPage = getLinesOfPerPage() - 1;
    if (nPage < 1) {
        nPage = 1;
    }
    if (m_pVertScrollBar) {
        m_pVertScrollBar->setScrollInfo(0, getRowCount(), nPage, m_nFirstVisibleRow, 1, bRedraw);
    }
}

void CSkinListView::setHorzScrollInfo(bool bRedraw) {
    int nWidth = 0;

    for (size_t i = 0; i < m_vHeading.size(); i++) {
        nWidth += m_vHeading[i]->nWidth;
    }

    if (m_pHorzScrollBar) {
        m_pHorzScrollBar->setScrollInfo(0, nWidth, m_rcContent.width(), m_pHorzScrollBar->getScrollPos(), 10, bRedraw);
    }
}


void CSkinListView::sendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol) {
    CSkinListCtrlEventNotify event(this, nClickedRow, nClickedCol);

    event.cmd = cmd;
    m_pSkin->dispatchUIObjNotify(&event);
}

CSkinListView::HitTestArea CSkinListView::hitTest(CPoint &point, int &nColumnIndex) {
    nColumnIndex = 0;

    if (!isPtIn(point)) {
        return HTA_NONE;
    }

    if (m_bDrawHeader && point.y - m_rcContent.top < m_nHeaderHeight) {
        int x = point.x - m_rcContent.left;
        if (m_pHorzScrollBar) {
            x += m_pHorzScrollBar->getScrollPos();
        }

        for (size_t i = 0; i < m_vHeading.size(); i++) {
            if (m_vHeading[i]->nWidth == 0) {
                continue;
            }

            if (x > m_vHeading[i]->nWidth) {
                x -= m_vHeading[i]->nWidth;
                continue;
            }

            int RESIZE_RANGE = 5;

            if (m_vHeading[i]->nWidth < 10) {
                RESIZE_RANGE = 2;
            }

            nColumnIndex = (int)i;

            if (x <= RESIZE_RANGE) {
                // Near to next column
                if (x * 2 > m_vHeading[i]->nWidth) {
                    return HTA_HEADER_SPLIT;
                }

                if (nColumnIndex == 0) {
                    return HTA_HEADER;
                }

                nColumnIndex = (int)i - 1;
                return HTA_HEADER_SPLIT;
            } else if (m_vHeading[i]->nWidth - x <= RESIZE_RANGE) {
                return HTA_HEADER_SPLIT;
            } else {
                return HTA_HEADER;
            }
        }

        return HTA_HEADER;
    }

    return HTA_ROW;
}

void CSkinListView::drawHeader(CRawGraph *canvas) {
    int x = m_rcContent.left;
    int y = m_rcContent.top;

    if (m_pHorzScrollBar) {
        x -= m_pHorzScrollBar->getScrollPos();
    }

    for (int i = 0; i < (int)m_vHeading.size(); i++) {
        CColHeader *pColHeader = m_vHeading[i];
        if (pColHeader->nWidth <= 0) {
            continue;
        }

        if (x + pColHeader->nWidth < m_rcContent.left) {
            x += pColHeader->nWidth;
            continue;
        }

        CSFImage *pImageHeader = nullptr, *pImageSortFlag = nullptr;
        if (m_nSortedCol == i) {
            if (m_bAscending) {
                pImageSortFlag = &m_imageAscending;
            } else {
                pImageSortFlag = &m_imageDescending;
            }
            pImageHeader = &m_imageSortedHeader;
        } else {
            pImageHeader = &m_imageHeader;
        }

        // draw background image.
        if (pImageHeader->isValid()) {
            pImageHeader->stretchBlt(canvas, x, y, pColHeader->nWidth, m_nHeaderHeight);
        }

        // sort flag
        if (pImageSortFlag && pImageSortFlag->isValid()) {
            pImageSortFlag->blt(canvas, x + pColHeader->nWidth - 2 - pImageSortFlag->m_cx,
                y + (m_nHeaderHeight - pImageSortFlag->height()) / 2);
        }

        // draw Head Text
        CRect rc(x + 3, y + 1, x + pColHeader->nWidth - 5, y + m_nHeaderHeight - 2);
        canvas->drawText(pColHeader->strTitle.c_str(), pColHeader->strTitle.size(), rc, DT_VCENTER);

        x += pColHeader->nWidth;
        if (x > m_rcContent.right) {
            break;
        }

        // draw line
        canvas->line(x - 1, y, x - 1, y + m_nHeaderHeight);
    }

    // draw background image.
    if (m_imageHeader.isValid() && x < m_rcContent.right) {
        m_imageHeader.stretchBlt(canvas, x, y, m_rcContent.right - x, m_nHeaderHeight);
    }

    canvas->line(m_rcContent.left, y + m_nHeaderHeight - 1, m_rcContent.right, y + m_nHeaderHeight - 1);
}

void CSkinListView::invalidateItem(int nItem) {
    if (nItem >= m_nFirstVisibleRow && nItem <= getLastVisibleRow()) {
        invalidate();
    }
}

bool CSkinListView::isClickedOn(int row, int col, CColHeader *pHeader, int x, int y) {
    CColHeader *header = m_vHeading[col];
    if (header->colType == CColHeader::TYPE_IMAGE) {
        CRawImage *image = m_dataSource->getCellImage(row, col);
        if (image != nullptr) {
            return x >= 0 && x < image->width() && y >= 0 && y < image->height();
        }
    } else {
        return true;
    }

    return false;
}

void CSkinListView::drawCell(int row, int col, CRect &rcCell, CRawGraph *canvas, CColor &clrText) {
    int colType = m_vHeading[col]->colType;
    if (colType == CColHeader::TYPE_TEXT) {
        cstr_t text = m_dataSource->getCellText(row, col);
        if (text != nullptr) {
            drawCellText(text, rcCell, canvas, clrText);
        }
    } else if (colType == CColHeader::TYPE_IMAGE) {
        CRawImage *image = m_dataSource->getCellImage(row, col);
        if (image != nullptr) {
            drawCellImage(*image, rcCell, canvas);
        }
    } else {
        assert(0 && "Undefined column type.");
    }
}

void CSkinListView::drawCellText(cstr_t text, CRect &rcCell, CRawGraph *canvas, CColor &clrText) {
    if (m_font.isOutlined()) {
        canvas->drawTextOutlined(text, -1, rcCell, clrText,
            m_font.getColorOutlined(), DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
    } else {
        canvas->setTextColor(clrText);
        canvas->drawText(text, -1, rcCell, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
    }
}

void CSkinListView::drawCellImage(CRawImage &image, CRect &rcCell, CRawGraph *canvas) {
    if (image.isValid()) {
        CRawGraph::CClipBoxAutoRecovery cbar(canvas);

        canvas->setClipBoundBox(rcCell);

        int yDest = rcCell.top + (rcCell.bottom - rcCell.top - image.height()) / 2;

        image.blt(canvas, rcCell.left, yDest);
    }
}

void CSkinListView::draw(CRawGraph *canvas) {
    CSkinScrollFrameCtrlBase::draw(canvas);

    int nImageCx = 0;

    if (m_font.isGood()) {
        canvas->setFont(m_font.getFont());
    }

    if (m_imageList.isValid()) {
        nImageCx = m_imageList.getIconCx();
    }

    canvas->setTextColor(m_font.getTextColor(m_enable));

    CRawGraph::CClipBoxAutoRecovery autoCBR(canvas);
    canvas->setClipBoundBox(m_rcContent);

    canvas->setPen(m_penLine);

    // draw background color
    if (isUseParentBg()) {
        m_pContainer->redrawBackground(canvas, m_rcContent);
    } else {
        int alpha;
        CColor clrBg1, clrBg2;

        if (m_translucencyWithSkin) {
            alpha = int(m_pSkin->m_nCurTranslucencyAlpha);
        } else {
            alpha = 255;
        }

        clrBg1 = getColor(CN_BG);
        clrBg1.setAlpha(alpha);

        clrBg2 = getColor(CN_ALTER_BG);
        clrBg2.setAlpha(alpha);

        CRect rcItem = m_rcContent;
        if (m_bDrawHeader) {
            rcItem.top += m_nHeaderHeight;
        }
        rcItem.bottom = rcItem.top + m_nLineHeight;
        bool bFlag = true;
        while (rcItem.top < m_rcContent.bottom) {
            if (bFlag) {
                canvas->fillRect(&rcItem, clrBg1);
            } else {
                canvas->fillRect(&rcItem, clrBg2);
            }
            bFlag = !bFlag;
            rcItem.top = rcItem.bottom;
            rcItem.bottom += m_nLineHeight;
        }
    }

    CRect rc = m_rcContent;
    rc.left += m_nXMargin;
    rc.right -= m_nXMargin;

    if (m_bDrawHeader) {
        drawHeader(canvas);
        rc.top += m_nHeaderHeight;
    }

    if (m_nFirstVisibleRow >= (int)getRowCount()) {
        m_nFirstVisibleRow = (int)getRowCount() - rc.height() / m_nLineHeight;
    }
    if (m_nFirstVisibleRow < 0) {
        m_nFirstVisibleRow = 0;
    }

    int y = rc.top;
    int nClrName;
    for (int i = m_nFirstVisibleRow; i < (int)getRowCount(); i++) {
        int x = rc.left;
        if (m_pHorzScrollBar) {
            x -= m_pHorzScrollBar->getScrollPos();
        }

        if (m_dataSource->isRowSelected(i)) {
            CRect rc(m_rcContent.left, y, m_rcContent.left + m_rcContent.width(), y + m_nLineHeight);
            int alpha;
            CColor clr = getColor(CN_SEL_BG);

            if (m_translucencyWithSkin) {
                alpha = (int)(m_pSkin->m_nCurTranslucencyAlpha);
            } else {
                alpha = 255;
            }

            clr.setAlpha(alpha);

            canvas->fillRect(&rc, clr);
            nClrName = CN_SEL_TEXT;
        } else {
            nClrName = CN_TEXT;
        }

        CRect rcItem;
        rcItem.left = x;
        rcItem.top = y;
        rcItem.bottom = y + m_nLineHeight;
        for (int k = 0; k < (int)m_vHeading.size(); k++) {
            rcItem.right = rcItem.left + m_vHeading[k]->nWidth;
            if (k == 0 && nImageCx > 0) {
                // draw icon.
                if (nImageCx > 0 && rcItem.width() > 0) {
                    int nImageIndex = m_dataSource->getItemImageIndex(i);
                    if (nImageIndex >= 0) {
                        m_imageList.draw(canvas, nImageIndex, x, y + (m_nLineHeight - m_imageList.getIconCy()) / 2);
                    }
                    rcItem.left += nImageCx + m_imageListRightSpace;
                }
            }
            if (rcItem.right < m_rcContent.left) {
                rcItem.left = rcItem.right;
                continue;
            }

            if (rcItem.right > rc.right) {
                rcItem.right = rc.right;
            }

            drawCell(i, k, rcItem, canvas, getColor(nClrName));
            if (rcItem.right >= rc.right) {
                break;
            }
            rcItem.left = rcItem.right;
        }
        y += m_nLineHeight;
        if (y >= rc.bottom) {
            break;
        }
    }

    // draw Border
    if (m_bDrawBorder) {
        canvas->rectangle(m_rcContent.left, m_rcContent.top, m_rcContent.width(), m_rcContent.height());
    }
}


bool CSkinListView::onLButtonDown(uint32_t nFlags, CPoint point) {
    if (CSkinScrollFrameCtrlBase::onLButtonDown(nFlags, point)) {
        return true;
    }

    int nColumnIndex;
    HitTestArea hta = hitTest(point, nColumnIndex);
    if (hta == HTA_HEADER_SPLIT) {
        // Drag to resize column width.
        m_pSkin->setCaptureMouse(this);

        m_nResizingCol = nColumnIndex;
        m_ptStartDrag = point;
        m_bAdjustColWidth = true;
        setCursor(m_curResizeCol);
        return true;
    }

    m_bClickBtDown = true;

    CPaintUpdater updater(this);
    CSelChangedCheck selChecher(this);

    CPoint ptAtItemCell;
    int nClickedRow = -1, nClickedCol = -1;
    bool bItemClickable;
    getRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
    if (bItemClickable) {
        setClickCursor();
        return true;
    }

    m_nEndSelRow = nClickedRow;
    if (nFlags & MK_SHIFT) {
        selectRangeRow(m_nBegSelRow, m_nEndSelRow);
    } else {
        m_nBegSelRow = m_nEndSelRow;
        reverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);
    }

    if (m_nEndSelRow != -1) {
        makeSureRowVisible(m_nEndSelRow);
    }

    if (selChecher.isSelectChanged()) {
        updater.setUpdateFlag(true);
        sendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED, nClickedRow, nClickedCol);
    }

    return true;
}


bool CSkinListView::onRButtonDown(uint32_t nFlags, CPoint point) {
    if (CSkinScrollFrameCtrlBase::onRButtonDown(nFlags, point)) {
        return true;
    }

    int nColumnIndex;
    HitTestArea hta = hitTest(point, nColumnIndex);
    if (hta != HTA_ROW) {
        return true;
    }

    int nClickedRow = -1, nClickedCol = -1;
    getRowColAtPoint(point, nClickedRow, nClickedCol);
    if (nClickedRow != -1) {
        if (m_dataSource->isRowSelected(nClickedRow)) {
            return true;
        }
    }

    CPaintUpdater updater(this);
    CSelChangedCheck selChecher(this);

    m_nEndSelRow = nClickedRow;
    m_nBegSelRow = m_nEndSelRow;
    reverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);

    if (m_nEndSelRow != -1) {
        makeSureRowVisible(m_nEndSelRow);
    }

    if (selChecher.isSelectChanged()) {
        updater.setUpdateFlag(true);
        sendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED, nClickedRow, nClickedCol);
    }

    return false;
}


bool CSkinListView::onRButtonUp(uint32_t nFlags, CPoint point) {
    if (CSkinScrollFrameCtrlBase::onRButtonUp(nFlags, point)) {
        return true;
    }

    sendNotifyEvent(CSkinListCtrlEventNotify::C_RBTN_CLICK);

    return false;
}


bool CSkinListView::onLButtonDblClk(uint32_t nFlags, CPoint point) {
    if (CSkinScrollFrameCtrlBase::onLButtonDblClk(nFlags, point)) {
        return true;
    }

    CPaintUpdater updater(this);
    CSelChangedCheck selChecher(this);

    int nClickedRow = -1, nClickedCol = -1;
    getRowColAtPoint(point, nClickedRow, nClickedCol);
    m_nEndSelRow = nClickedRow;

    m_nBegSelRow = m_nEndSelRow;
    reverseSelectStatOfRow((nFlags & MK_CONTROL) != MK_CONTROL, m_nEndSelRow);

    updater.setUpdateFlag(selChecher.isSelectChanged());

    if (m_nEndSelRow != -1) {
        makeSureRowVisible(m_nEndSelRow);
    }

    sendNotifyEvent(CSkinListCtrlEventNotify::C_DBL_CLICK, nClickedRow, nClickedCol);

    return true;
}


void CSkinListView::onKeyDown(uint32_t nChar, uint32_t nFlags) {
    if (getRowCount() == 0) {
        return;
    }

    if (nChar == VK_RETURN) {
        sendNotifyEvent(CSkinListCtrlEventNotify::C_ENTER);
        return;
    } else if (nChar == VK_DELETE) {
        sendNotifyEvent(CSkinListCtrlEventNotify::C_KEY_DELETE);
        return;
    } else if (nChar == 'A') {
        bool bCtrl = isModifierKeyPressed(MK_CONTROL, nFlags);

        if (bCtrl) {
            // select all
            CSelChangedCheck selChecher(this);

            selectRangeRow(0, getRowCount() - 1);
            if (selChecher.isSelectChanged()) {
                invalidate();
            }
        }
        return;
    } else if (nChar == 'I') {
        bool bCtrl = isModifierKeyPressed(MK_CONTROL, nFlags);

        if (bCtrl) {
            // invert selection
            CSelChangedCheck selChecher(this);

            for (int i = 0; i < (int)getRowCount(); i++) {
                m_dataSource->setRowSelectionState(i, !m_dataSource->isRowSelected(i));
            }
            invalidate();
        }
        return;
    }

    bool bShift = isModifierKeyPressed(MK_SHIFT, nFlags);

    if (nChar != VK_UP && nChar != VK_DOWN && nChar != VK_PRIOR && nChar != VK_NEXT
        && nChar != VK_HOME && nChar != VK_END) {
        return;
    }

    CPaintUpdater updater(this);
    CSelChangedCheck selChecher(this);

    switch (nChar) {
    case VK_UP:
        {
            //             bool        bAlt = isKeyPressed(VK_MENU);
            //             if (bAlt)
            //             {
            //                 updater.setUpdateFlag(offsetAllSelectedRow(false));
            //                 return;
            //             }

            if (m_nEndSelRow == -1) {
                m_nEndSelRow = m_nFirstVisibleRow;
            } else if (m_nEndSelRow > 0) {
                m_nEndSelRow--;
            }

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }

            makeSureRowVisible(m_nEndSelRow);
        }
        break;
    case VK_DOWN:
        {
            //             bool        bAlt = isKeyPressed(VK_MENU);
            //             if (bAlt)
            //             {
            //                 updater.setUpdateFlag(offsetAllSelectedRow(true));
            //                 return;
            //             }

            if (m_nEndSelRow == -1) {
                m_nEndSelRow = m_nFirstVisibleRow;
            } else if (m_nEndSelRow < (int)getRowCount() - 1) {
                m_nEndSelRow++;
            }

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }

            makeSureRowVisible(m_nEndSelRow);
        }
        break;
    case VK_PRIOR:
        {
            if (m_nEndSelRow == -1) {
                m_nEndSelRow = m_nFirstVisibleRow;
            } else if (m_nEndSelRow > 0) {
                if (m_nEndSelRow != m_nFirstVisibleRow) {
                    m_nEndSelRow = m_nFirstVisibleRow;
                } else {
                    m_nEndSelRow = m_nFirstVisibleRow - (getLinesOfPerPage() - 1);
                    if (m_nEndSelRow == m_nFirstVisibleRow) {
                        m_nEndSelRow--;
                    }
                    if (m_nEndSelRow < 0) {
                        m_nEndSelRow = 0;
                    }
                }
            }

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }

            makeSureRowVisible(m_nEndSelRow);
        }
        break;
    case VK_NEXT:
        {
            if (m_nEndSelRow == -1) {
                m_nEndSelRow = m_nFirstVisibleRow;
            } else if (m_nEndSelRow < (int)getRowCount() - 1) {
                if (m_nEndSelRow != getLastVisibleRow()) {
                    m_nEndSelRow = getLastVisibleRow();
                } else {
                    m_nEndSelRow = getLastVisibleRow() + getLinesOfPerPage() - 1;
                    if (m_nEndSelRow == getLastVisibleRow()) {
                        m_nEndSelRow++;
                    }
                    if (m_nEndSelRow >= (int)getRowCount()) {
                        m_nEndSelRow = getRowCount() - 1;
                    }
                    if (m_nEndSelRow < 0) {
                        m_nEndSelRow = 0;
                    }
                }
            }

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }

            makeSureRowVisible(m_nEndSelRow);
        }
        break;
    case VK_HOME:
        {
            m_nEndSelRow = 0;
            m_nFirstVisibleRow = 0;

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }
        }
        break;
    case VK_END:
        {
            m_nEndSelRow = getRowCount() - 1;

            if (bShift) {
                selectRangeRow(m_nBegSelRow, m_nEndSelRow);
            } else {
                m_nBegSelRow = m_nEndSelRow;
                reverseSelectStatOfRow(true, m_nEndSelRow);
            }

            makeSureRowVisible(m_nEndSelRow);
        }
        break;
    }

    if (selChecher.isSelectChanged()) {
        updater.setUpdateFlag(true);
        sendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
    }
}


bool CSkinListView::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (m_bAdjustColWidth) {
        m_pSkin->releaseCaptureMouse(this);

        m_bAdjustColWidth = false;
        setCursor(m_curResizeCol);
        return true;
    }

    if (CSkinScrollFrameCtrlBase::onLButtonUp(nFlags, point)) {
        return true;
    }

    invalidate();

    if (m_bClickBtDown) {
        m_bClickBtDown = false;

        CPoint ptAtItemCell;
        int nClickedRow = -1, nClickedCol = -1;
        bool bItemClickable;
        getRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
        if (bItemClickable) {
            setClickCursor();

            CSkinListCtrlEventNotify event(this, nClickedRow, nClickedCol);
            event.cmd = CSkinListCtrlEventNotify::C_CLICK_CMD;
            m_pSkin->dispatchUIObjNotify(&event);
        } else {
            sendNotifyEvent(CSkinListCtrlEventNotify::C_CLICK, nClickedRow, nClickedCol);
        }
    }

    return true;
}

bool CSkinListView::onMouseDrag(CPoint point) {
    if (m_bAdjustColWidth && m_nResizingCol < (int)m_vHeading.size()) {
        setCursor(m_curResizeCol);

        int nOffset = point.x - m_ptStartDrag.x;
        if (nOffset + m_vHeading[m_nResizingCol]->nWidth >= 0) {
        } else if (m_vHeading[m_nResizingCol]->nWidth > 0) {
            nOffset = - m_vHeading[m_nResizingCol]->nWidth;
        } else {
            nOffset = 0;
        }

        if (nOffset != 0) {
            m_vHeading[m_nResizingCol]->nWidth += nOffset;
            m_ptStartDrag.x += nOffset;
            setHorzScrollInfo(true);
            invalidate();
        }

        return true;
    } else {
        return CSkinScrollFrameCtrlBase::onMouseDrag(point);
    }
}

bool CSkinListView::onMouseMove(CPoint point) {
    if (CSkinScrollFrameCtrlBase::onMouseMove(point)) {
        return true;
    }

    int nColumnIndex;
    HitTestArea hta = hitTest(point, nColumnIndex);
    if (hta == HTA_HEADER_SPLIT) {
        // set cursor of resize column width.
        setCursor(m_curResizeCol);
    } else if (hta == HTA_ROW && m_bHoverMouseSel) {
        // Mouse hover to change selected.
        CSelChangedCheck selChecher(this);

        int nClickedRow = -1, nClickedCol = -1;
        getRowColAtPoint(point, nClickedRow, nClickedCol, nullptr);
        m_nBegSelRow = m_nEndSelRow = nClickedRow;
        selectRangeRow(m_nBegSelRow, m_nEndSelRow);

        if (selChecher.isSelectChanged()) {
            sendNotifyEvent(CSkinListCtrlEventNotify::C_SEL_CHANGED);
            invalidate();
        }
    } else {
        int nClickedRow = -1, nClickedCol = -1;
        CPoint ptAtItemCell;
        bool bItemClickable;
        getRowColAtPoint(point, nClickedRow, nClickedCol, &ptAtItemCell, &bItemClickable);
        if (bItemClickable) {
            setClickCursor();
        }
    }

    return true;
}

int CSkinListView::getRowCount() const {
    if (m_dataSource != nullptr) {
        return m_dataSource->getRowCount();
    } else {
        return 0;
    }
}
