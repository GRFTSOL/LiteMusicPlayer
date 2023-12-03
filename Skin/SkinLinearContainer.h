#pragma once

#include "SkinContainer.h"


class CSkinLinearContainer : public CSkinContainer {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
    CSkinLinearContainer();
    virtual ~CSkinLinearContainer();

    void recalculateUIObjSizePos(CUIObject *pObj) override;

    void onMeasureSizeByContent() override;
    void onSize() override;

    bool setProperty(cstr_t szProperty, cstr_t szValue) override;

    int getItemsMinSumHeight();

protected:
    void onSizeForVert();
    void onSizeForHorz();

    struct Item {
        uint16_t                    minSize, size;
        uint16_t                    weight;

        void set(uint16_t minSize, uint16_t size, uint16_t weight) {
            this->minSize = minSize;
            this->size = size;
            this->weight = weight;
        }
    };
    typedef vector<Item>    VecItems;

    static void zoomWeightable(VecItems &vItems, int nSize);

    static void align(VecItems &vItems, int nSize);

    void uIObjsToItems(VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, VecItems &vItems, int &totalMinSize, int &totalSize, int &totalFixedSize);

    void resizeFromItemsToUIObjs(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);
    void resizeFromItemsToUIObjsForVert(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);
    void resizeFromItemsToUIObjsForHorz(VecItems &vItems, VecUIObjects::iterator itBeg, VecUIObjects::iterator itEnd, int startPos);

    virtual void onSetChildVisible(CUIObject *pChild, bool bVisible, bool bRedraw) override;

protected:
    friend class SkinLinearContainer_ZoomFromWeightable_Test;

    bool                        m_bVertical;
    int                         m_nSeparatorThickness;

};
