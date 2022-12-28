#ifndef Skin_SkinListCtrl_h
#define Skin_SkinListCtrl_h

#pragma once

class CSkinListCtrl;

#include "SkinListView.h"


class CSkinListCtrl : public CSkinListView, public IListViewDataSource {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinListView)
public:
    class Item {
    public:
        virtual ~Item() { }
    };

    class ItemString : public Item {
    public:
        ItemString() { }
        ItemString(cstr_t str) { text = str; }
        string                      text;
    };

    class ItemImage : public Item {
    public:
        ItemImage() { }
        CRawImage                   image;
    };

    struct Row {
        Row(int nImageIndex, uint32_t nItemData) {
            dwFlags = 0;

            this->nItemData = nItemData;
            this->nImageIndex = nImageIndex;
        }

        ~Row() {
            vector<Item*>::iterator it;
            for (it = vItems.begin(); it != vItems.end(); ++it) {
                delete (*it);
            }
            vItems.clear();
        }

        enum {
            F_SELECTED                  = 1,
        };

        vector<Item                 *>            vItems;
        uint32_t                    nItemData;
        int                         nImageIndex;
        uint32_t                    dwFlags;

        bool isSelected() { return (dwFlags & F_SELECTED) == F_SELECTED; }
        void setSelected(bool bSelected)
            { if (bSelected) dwFlags |= F_SELECTED; else dwFlags &= ~F_SELECTED; }

        Item *getSubItem(int n) {
            if (n >= (int)vItems.size() || n < 0) {
                return nullptr;
            }
            return vItems[n];
        }

    };

public:
    CSkinListCtrl();
    virtual ~CSkinListCtrl();

    virtual void addColumn(cstr_t szCol, int nWidth, int colType = CColHeader::TYPE_TEXT, bool bClickable = false, int drawTextAligns = DT_LEFT) override;

    virtual void addImageColumn(cstr_t szCol, int nWidth, bool bClickable = false) { addColumn(szCol, nWidth, CColHeader::TYPE_IMAGE, bClickable); }

    int getItemText(int nItem, int nSubItem, char * lpszText, int nLen);
    int getItemText(int nItem, int nSubItem, string &strText);
    virtual bool setItemText(int nItem, int nSubItem, cstr_t lpszText, bool bRedraw = true);

    virtual bool setItemImage(int nItem, int nSubItem, RawImageData *image, bool bRedraw = true);

    virtual int insertItem(int nItem, cstr_t lpszItem, int nImageIndex = 0, uint32_t nItemData = 0, bool bRedraw = true);
    uint32_t getItemCount() const { return (uint32_t)m_vRows.size(); }

    bool deleteItem(int nItem, bool bRedraw = true);
    bool deleteSelectedItems(bool bRedraw = true);
    bool deleteAllItems(bool bRedraw = true);

    bool canOffsetSelectedRow(bool bDown = true);
    bool offsetAllSelectedRow(bool bDown = true);

    uint32_t getItemData(int nItem);
    void setItemData(int nItem, uint32_t nItemData);

    //
    // Methods for IListViewDataSource
    //

    virtual int getRowCount() const override;

    virtual bool isRowSelected(int row) const override;
    virtual void setRowSelectionState(int nIndex, bool bSelected) override;

    virtual int getItemImageIndex(int row) override;
    virtual bool setItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true) override;

    virtual cstr_t getCellText(int row, int col) override;
    virtual CRawImage *getCellImage(int row, int col) override;

protected:
    virtual Item *newItem(int nCol);

protected:
    typedef vector<Row *>                V_ROWS;

    friend class LC_ITEM;
    friend class LC_ITEMEX;
    friend class CPopupSkinListWnd;

    V_ROWS                      m_vRows;

};

#endif // !defined(Skin_SkinListCtrl_h)
