#include "SkinTypes.h"
#include "Skin.h"
#include "SkinLinearContainer.h"
#include "SkinResizableLinearContainer.h"
#include "api-js/JsResizableLinearContainer.hpp"


UIOBJECT_CLASS_NAME_IMP(CSkinResizableLinearContainer, "ResizableLinearContainer")

//////////////////////////////////////////////////////////////////////

CSkinResizableLinearContainer::CSkinResizableLinearContainer() {
    m_bDragWnd = false;

    m_cursorWE.loadStdCursor(Cursor::C_SIZEWE);
    m_cursorNS.loadStdCursor(Cursor::C_SIZENS);

    m_nThicknessOfResizer = 3;
}

CSkinResizableLinearContainer::~CSkinResizableLinearContainer() {
}

bool CSkinResizableLinearContainer::onMouseDrag(CPoint point) {
    if (m_bDragWnd) {
        setResizerCursor();

        // start to resizer drag now
        if (m_bVertical) {
            int offset = point.y - m_ptDragOld.y;
            dragResizer(offset);
            m_ptDragOld.y += offset;
        } else {
            int offset = point.x - m_ptDragOld.x;
            dragResizer(offset);
            m_ptDragOld.x += offset;
        }

        return true;
    }

    int nResizerArea = hitTestResizer(point);
    if (nResizerArea != -1) {
        setResizerCursor();
    }

    return CSkinLinearContainer::onMouseDrag(point);
}

bool CSkinResizableLinearContainer::onMouseMove(CPoint point) {
    int nResizerArea = hitTestResizer(point);
    if (nResizerArea != -1) {
        setResizerCursor();
    }

    return CSkinLinearContainer::onMouseMove(point);
}

bool CSkinResizableLinearContainer::onLButtonDown(uint32_t nFlags, CPoint point) {
    m_nCurrentResizerArea = hitTestResizer(point);
    if (m_nCurrentResizerArea == -1) {
        return CSkinLinearContainer::onLButtonDown(nFlags, point);
    }

    m_bDragWnd = true;
    m_ptDragOld = point;
    m_pSkin->setCaptureMouse(this);

    setResizerCursor();

    return true;
}

bool CSkinResizableLinearContainer::onLButtonUp(uint32_t nFlags, CPoint point) {
    if (!m_bDragWnd) {
        return CSkinLinearContainer::onLButtonUp(nFlags, point);
    }

    m_bDragWnd = false;
    m_pSkin->releaseCaptureMouse(this);

    setResizerCursor();

    return true;
}

bool CSkinResizableLinearContainer::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinLinearContainer::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "ResizerThickness")) {
        m_nThicknessOfResizer = atoi(szValue);
    } else {
        return false;
    }

    return true;
}

int CSkinResizableLinearContainer::hitTestResizer(CPoint pt) {
    if (!m_rcObj.ptInRect(pt)) {
        return -1;
    }

    for (int i = 0; i < (int)m_vUIObjs.size(); i++) {
        CUIObject *p = m_vUIObjs[i];

        if (m_bVertical) {
            if (pt.y < p->m_rcObj.top + p->getMargin().top) {
                return i - 1;
            } else if (pt.y < p->m_rcObj.bottom + p->getMargin().bottom) {
                return -1;
            }
        } else {
            if (pt.x < p->m_rcObj.left + p->getMargin().left) {
                return i - 1;
            } else if (pt.x < p->m_rcObj.right + p->getMargin().right) {
                return -1;
            }
        }
    }

    return -1;
}

void CSkinResizableLinearContainer::setResizerCursor() {
    if (m_bVertical) {
        setCursor(m_cursorNS);
    } else {
        setCursor(m_cursorWE);
    }
}

void CSkinResizableLinearContainer::dragResizer(int &offset) {
    assert(m_nCurrentResizerArea != -1);
    if (m_nCurrentResizerArea < 0 || m_nCurrentResizerArea >= (int)m_vUIObjs.size()) {
        offset = 0;
        return;
    }

    // Check whether and how much we can resize.
    int totalMinSize = 0, totalFixedSize = 0;
    VecItems vItemsStart;
    int totalSizeStart = 0;
    uIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.begin() + m_nCurrentResizerArea + 1, vItemsStart, totalMinSize, totalSizeStart, totalFixedSize);
    int offsetMin = totalMinSize - totalSizeStart;

    VecItems vItemsEnd;
    int totalSizeEnd = 0;
    uIObjsToItems(m_vUIObjs.begin() + m_nCurrentResizerArea + 1, m_vUIObjs.end(), vItemsEnd, totalMinSize, totalSizeEnd, totalFixedSize);
    int offsetMax = totalSizeEnd - totalMinSize;

    if (offset < offsetMin) {
        offset = offsetMin;
    } else if (offset > offsetMax) {
        offset = offsetMax;
    }

    if (offset == 0) {
        return;
    }

    // Drag to top or left
    zoomFromRight(vItemsStart, totalSizeStart + offset);

    // Drag to top or left
    zoomFromLeft(vItemsEnd, totalSizeEnd - offset);

    vItemsStart.insert(vItemsStart.end(), vItemsEnd.begin(), vItemsEnd.end());

    resizeFromItemsToUIObjs(vItemsStart, m_vUIObjs.begin(), m_vUIObjs.end(),
        m_bVertical ? m_rcContent.top : m_rcContent.left);

    invalidate();
}

void CSkinResizableLinearContainer::onCreate() {
    CSkinLinearContainer::onCreate();

    int totalMinSize = 0, totalFixedSize = 0, totalSizeStart = 0;
    VecItems vItems;
    uIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSizeStart, totalFixedSize);

    // Reload sizes of children
    VecStrings vSize;
    strSplit(getString("ChildrenSize", "").c_str(), ',', vSize);
    if (vSize.size() == vItems.size()) {
        int i = 0;
        for (VecItems::iterator it = vItems.begin();
        i < (int)vSize.size() && it != vItems.end(); i++, ++it)
            {
            if (vSize[i].empty()) {
                continue;
            }
            int size = atoi(vSize[i].c_str());
            (*it).size = size;
        }

        resizeFromItemsToUIObjs(vItems, m_vUIObjs.begin(), m_vUIObjs.end(),
            m_bVertical ? m_rcContent.top : m_rcContent.left);
    }
}

void CSkinResizableLinearContainer::onDestroy() {
    int totalMinSize = 0, totalFixedSize = 0, totalSizeStart = 0;
    VecItems vItems;
    uIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSizeStart, totalFixedSize);

    // save sizes of children
    string strSizes;
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        if (strSizes.size()) {
            strSizes += ',';
        }
        strSizes += itos((*it).size);
    }

    writeString("ChildrenSize", strSizes.c_str());

    CSkinLinearContainer::onDestroy();
}

void CSkinResizableLinearContainer::zoomFromRight(VecItems &vItems, int nSize) {
    if (vItems.size() == 0) {
        return;
    }

    int sizeTotal = 0;
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        sizeTotal += item.size;
    }

    int sizeDecrease = sizeTotal - nSize;
    if (sizeDecrease <= 0) {
        // Zoom in rightest
        vItems.back().size += -sizeDecrease;
        return;
    }

    // Zoom out one by one from right
    for (VecItems::reverse_iterator it = vItems.rbegin(); it != vItems.rend(); ++it) {
        Item &item = *it;
        if (sizeDecrease <= item.size - item.minSize) {
            // Zoom out this item can end the zoom out
            item.size -= sizeDecrease;
            break;
        }

        sizeDecrease -= item.size - item.minSize;
        item.size = item.minSize;
    }
}

void CSkinResizableLinearContainer::zoomFromLeft(VecItems &vItems, int nSize) {
    if (vItems.size() == 0) {
        return;
    }

    int sizeTotal = 0;
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        sizeTotal += item.size;
    }

    int sizeDecrease = sizeTotal - nSize;
    if (sizeDecrease <= 0) {
        // Zoom in left one
        vItems.front().size += -sizeDecrease;
        return;
    }

    // Zoom out one by one from left
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        if (sizeDecrease <= item.size - item.minSize) {
            // Zoom out this item can end the zoom out
            item.size -= sizeDecrease;
            break;
        }

        sizeDecrease -= item.size - item.minSize;
        item.size = item.minSize;
    }
}

void CSkinResizableLinearContainer::zoomAll(VecItems &vItems, int nSize) {
    if (vItems.size() == 0) {
        return;
    }

    int sizeToAlloc = nSize;
    int totalMinSize = 0;
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;

        sizeToAlloc -= item.minSize;

        if (item.weight == 0) {
            totalMinSize += item.minSize;
        }
    }
    assert(totalMinSize > 0);

    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        if (item.weight > 0) {
            item.size = item.minSize;
        } else if (item.minSize == 0) {
            item.size = 10;
        } else {
            item.size = item.minSize + sizeToAlloc * item.minSize / totalMinSize;
        }
    }
}

JsValue CSkinResizableLinearContainer::getJsObject(VMContext *ctx) {
    return ctx->runtime->pushObject(new JsResizableLinearContainer(this));
}

void CSkinResizableLinearContainer::setMode(cstr_t modeName) {
    SizedString mode(modeName);
    if (m_vUIObjs.size() >= 2) {
        if (mode.equal("left")) {
            m_vUIObjs[0]->setVisible(true, false);
            m_vUIObjs[1]->setVisible(false, false);
        } else if (mode.equal("right")) {
            m_vUIObjs[1]->setVisible(true, false);
            m_vUIObjs[0]->setVisible(false, false);
        } else if (mode.equal("all")) {
            m_vUIObjs[1]->setVisible(true, false);
            m_vUIObjs[0]->setVisible(true, false);
        }

        onSize();
        invalidate();
    }
}

cstr_t CSkinResizableLinearContainer::getMode() const {
    if (m_vUIObjs.size() >= 2) {
        if (m_vUIObjs[0]->isVisible()) {
            if (m_vUIObjs[1]->isVisible()) {
                return "all";
            }
            return "left";
        } else {
            return "right";
        }
    }

    return "all";
}
