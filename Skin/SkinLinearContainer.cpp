#include "SkinTypes.h"
#include "Skin.h"
#include "SkinLinearContainer.h"


UIOBJECT_CLASS_NAME_IMP(CSkinLinearContainer, "LinearContainer")

//////////////////////////////////////////////////////////////////////

CSkinLinearContainer::CSkinLinearContainer() {
    m_bVertical = true;
    m_nSeparatorThickness = 0;
}

CSkinLinearContainer::~CSkinLinearContainer() {
}

void CSkinLinearContainer::recalculateUIObjSizePos(CUIObject *pObj) {
    m_pContainer->recalculateUIObjSizePos(this);
}

void CSkinLinearContainer::onMeasureSizeByContent() {
    FORMULA_VAR     vars[] = {
        {'w', m_rcObj.width()},
        {'h', m_rcObj.height()},
        {0, 0}
    };

    uint32_t requestedHeight = 0;
    uint32_t requestedWidth = 0;
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *p = m_vUIObjs[i];
        if (!p->isVisible()) {
            continue;
        }

        p->onMeasureSizePos(vars);
        if (p->getWeight() > 0) {
            DBG_LOG2("Parent LinearContainer has wrap content property, the weight of child can't take effect: %s, %d.", p->m_strName.c_str(), p->getWeight());
        }

        if (m_bVertical) {
            requestedHeight += p->m_rcObj.height();
            requestedHeight += p->getMargin().top + p->getMargin().bottom;

            int w = p->m_rcObj.width() + p->getMargin().left + p->getMargin().right;
            if (w > (int)requestedWidth) {
                requestedWidth = w;
            }
        } else {
            requestedWidth += p->m_rcObj.width();
            requestedWidth += p->getMargin().left + p->getMargin().right;

            int h = p->m_rcObj.height() + p->getMargin().top + p->getMargin().bottom;
            if (h > (int)requestedHeight) {
                requestedHeight = h;
            }
        }
    }

    m_rcObj.right = m_rcObj.left + requestedWidth;
    m_rcObj.bottom = m_rcObj.top + requestedHeight;
}

void CSkinLinearContainer::onSize() {
    CUIObject::onSize();

    FORMULA_VAR     vars[] = {
        {'w', m_rcObj.width()},
        {'h', m_rcObj.height()},
        {0, 0}
    };

    // Measure child size position
    for (uint32_t i = 0; i < m_vUIObjs.size(); i++) {
        CUIObject *p = m_vUIObjs[i];
        if (!p->isVisible()) {
            continue;
        }

        p->onMeasureSizePos(vars);
    }

    VecItems vItems;
    int totalMinSize = 0, totalFixedSize = 0, totalSize = 0;
    uIObjsToItems(m_vUIObjs.begin(), m_vUIObjs.end(), vItems, totalMinSize, totalSize, totalFixedSize);

    int size;
    if (m_bVertical) {
        size = m_rcObj.height() - totalFixedSize;
    } else {
        size = m_rcObj.width() - totalFixedSize;
    }

    zoomWeightable(vItems, size);

    resizeFromItemsToUIObjs(vItems, m_vUIObjs.begin(), m_vUIObjs.end(),
        m_bVertical ? m_rcContent.top : m_rcContent.left);
}

bool CSkinLinearContainer::setProperty(cstr_t szProperty, cstr_t szValue) {
    if (CSkinContainer::setProperty(szProperty, szValue)) {
        return true;
    }

    if (isPropertyName(szProperty, "Orientation")) {
        m_bVertical = isPropertyName(szValue, "Vertical");
    } else if (isPropertyName(szProperty, "SeparatorThickness")) {
        m_nSeparatorThickness = atoi(szValue);
    } else {
        return false;
    }

    return true;
}

void CSkinLinearContainer::zoomWeightable(VecItems &vItems, int nSize) {
    if (vItems.size() == 0) {
        return;
    }

    int totalWeight = 0;
    int sizeToAlloc = nSize;
    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        if (item.weight > 0) {
            totalWeight += item.weight;
            sizeToAlloc -= item.minSize;
        } else {
            sizeToAlloc -= item.size;
        }
    }

    for (VecItems::iterator it = vItems.begin(); it != vItems.end(); ++it) {
        Item &item = *it;
        if (item.weight > 0 && totalWeight > 0) {
            item.size = item.minSize + sizeToAlloc * item.weight / totalWeight;
            if (item.size < 0) {
                item.size = 0;
            }
        }
    }
}

void CSkinLinearContainer::uIObjsToItems(VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, VecItems &vItems, int &totalMinSize, int &totalSize, int &totalFixedSize) {
    totalMinSize = 0;
    totalFixedSize = 0;
    totalSize = 0;

    for (VecUIObjects::iterator it = itBeg; it != itEnd; ++it) {
        CUIObject *p = *it;
        if (!p->isVisible()) {
            continue;
        }

        totalFixedSize += m_nSeparatorThickness;

        // Calculate totalFixedSize
        if (m_bVertical) {
            if (p->isFixedHeight()) {
                totalFixedSize += p->m_rcObj.height() + p->getMargin().top + p->getMargin().bottom;
                continue;
            }
        } else {
            if (p->isFixedWidth()) {
                totalFixedSize += p->m_rcObj.width() + p->getMargin().left + p->getMargin().right;
                continue;
            }
        }

        Item item;

        item.weight = p->getWeight();
        if (m_bVertical) {
            item.minSize = p->getMinHeight();
            item.size = p->m_rcObj.height() - p->m_rcPadding.top - p->m_rcPadding.bottom;
            totalFixedSize += p->getMargin().top + p->getMargin().bottom;
        } else {
            item.minSize = p->getMinWidth();
            item.size = p->m_rcObj.width() - p->m_rcPadding.left - p->m_rcPadding.right;
            totalFixedSize += p->getMargin().left + p->getMargin().right;
        }

        totalMinSize += item.minSize;
        totalSize += item.size;

        vItems.push_back(item);
    }

    // decrease the extra item.
    totalFixedSize -= m_nSeparatorThickness;
}

int CSkinLinearContainer::getItemsMinSumHeight() {
    int n = 0;

    for (auto p : m_vUIObjs) {
        if (!p->isVisible()) {
            continue;
        }

        n += m_nSeparatorThickness;

        assert(m_bVertical);
        if (p->isFixedHeight()) {
            n += p->m_rcObj.height() + p->getMargin().top + p->getMargin().bottom;
        } else {
            n += p->getMinHeight() + p->getMargin().top + p->getMargin().bottom;
        }
    }

    if (n > m_nSeparatorThickness) {
        n -= m_nSeparatorThickness;
    }

    return n;
}

void CSkinLinearContainer::resizeFromItemsToUIObjs(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos) {
    // Calculate size position for every UIObject
    if (m_bVertical) {
        resizeFromItemsToUIObjsForVert(vItems, itBeg, itEnd, startPos);
    } else {
        resizeFromItemsToUIObjsForHorz(vItems, itBeg, itEnd, startPos);
    }
}

void CSkinLinearContainer::resizeFromItemsToUIObjsForVert(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos) {
    // Calculate size position for every UIObject
    int pos = startPos;
    VecItems::iterator itItem = vItems.begin();
    for (VecUIObjects::iterator it = itBeg; it != itEnd; ++it) {
        CUIObject *p = *it;
        if (!p->isVisible()) {
            continue;
        }

        // Calculate height
        int height;
        if (p->isFixedHeight()) {
            height = p->m_rcObj.height();
        } else {
            if (itItem == vItems.end()) {
                assert(0);
                DBG_LOG0("vItems shall NOT end now.");
                break;
            }
            Item &item = *itItem;
            height = item.size;
            ++itItem;
            // set the size value back to UIObject
            p->m_formHeight.setFormula(height);
        }
        p->m_rcObj.top = pos + p->getMargin().top;
        p->m_rcObj.bottom = p->m_rcObj.top + height;
        pos = p->m_rcObj.bottom + p->getMargin().bottom;

        // Calculate width
        int width;
        if (p->getLayoutParams() & LAYOUT_WIDTH_MATCH_PARENT) {
            width = m_rcObj.width() - p->getMargin().left - p->getMargin().right;
        } else {
            width = p->m_rcObj.width();
        }

        // Calculate left and right
        if (p->getLayoutParams() & LAYOUT_ALIN_RIGHT) {
            p->m_rcObj.right = m_rcObj.right - p->getMargin().right;
            p->m_rcObj.left = p->m_rcObj.right - width - p->getMargin().left;
        } else if (p->getLayoutParams() & LAYOUT_ALIN_HORZ_CENTER) {
            p->m_rcObj.left = (m_rcObj.left + m_rcObj.right - width) / 2;
            p->m_rcObj.right += p->m_rcObj.left + width;
        } else {
            p->m_rcObj.left = m_rcObj.left + p->getMargin().right;
            p->m_rcObj.right = m_rcObj.left + width;
        }

        p->onSize();

        pos += m_nSeparatorThickness;
    }
}

void CSkinLinearContainer::resizeFromItemsToUIObjsForHorz(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos) {
    // Calculate size position for every UIObject
    int pos = startPos;
    VecItems::iterator itItem = vItems.begin();
    for (VecUIObjects::iterator it = itBeg; it != itEnd && itItem != vItems.end(); ++it) {
        CUIObject *p = *it;
        if (!p->isVisible()) {
            continue;
        }

        // Calculate width
        int width;
        if (p->isFixedWidth()) {
            width = p->m_rcObj.width();
        } else {
            Item &item = *itItem;
            width = item.size;
            ++itItem;
            // set the size value back to UIObject
            p->m_formWidth.setFormula(width);
        }
        p->m_rcObj.left = pos + p->getMargin().left;
        p->m_rcObj.right = p->m_rcObj.left + width;
        pos = p->m_rcObj.right + p->getMargin().right;

        // Calculate height
        int height;
        if (p->getLayoutParams() & LAYOUT_HEIGHT_MATCH_PARENT) {
            height = m_rcObj.height() - p->getMargin().top - p->getMargin().bottom;
        } else {
            height = p->m_rcObj.height();
        }

        // Calculate left and right
        if (p->getLayoutParams() & LAYOUT_ALIN_BOTTOM) {
            p->m_rcObj.bottom = m_rcObj.bottom - p->getMargin().bottom;
            p->m_rcObj.top = p->m_rcObj.bottom - height - p->getMargin().top;
        } else if (p->getLayoutParams() & LAYOUT_ALIN_VERT_CENTER) {
            p->m_rcObj.top = (m_rcObj.top + m_rcObj.bottom - height) / 2;
            p->m_rcObj.bottom += p->m_rcObj.top + height;
        } else {
            p->m_rcObj.top = m_rcObj.top + p->getMargin().bottom;
            p->m_rcObj.bottom = m_rcObj.top + height;
        }

        p->onSize();

        pos += m_nSeparatorThickness;
    }
}

void CSkinLinearContainer::onSetChildVisible(CUIObject *pChild, bool bVisible, bool bRedraw) {
    // Recalculate the size position.
    m_pContainer->recalculateUIObjSizePos(this);
    if (bRedraw) {
        invalidate();
    }
}

#if UNIT_TEST

#include "utils/unittest.h"


TEST(SkinLinearContainer, ZoomFromWeightable) {
    CSkinLinearContainer::VecItems items;
    CSkinLinearContainer::Item item;

    item.set(80, 100, 0);
    items.push_back(item);

    item.set(50, 50, 1);
    items.push_back(item);

    item.set(50, 50, 0);
    items.push_back(item);

    // Zoom out
    CSkinLinearContainer::zoomWeightable(items, 250);
    ASSERT_TRUE(items[0].size == 100);
    ASSERT_TRUE(items[1].size = 100);
    ASSERT_TRUE(items[2].size = 50);

    // Zoom to minimum
    CSkinLinearContainer::zoomWeightable(items, 200);
    ASSERT_TRUE(items[0].size == 100);
    ASSERT_TRUE(items[1].size = 50);
    ASSERT_TRUE(items[2].size = 50);

    // Zoom to less than minimum
    CSkinLinearContainer::zoomWeightable(items, 100);
    ASSERT_TRUE(items[0].size == 100);
    ASSERT_TRUE(items[1].size = 50);
    ASSERT_TRUE(items[2].size = 50);
}

#endif
