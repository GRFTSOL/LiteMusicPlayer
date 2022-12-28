#if !defined(__SKINLISTCTRLEX__H__)
#define __SKINLISTCTRLEX__H__

#pragma once


class CSkinListCtrlEx;


class CSkinListCtrlEx : public CSkinListCtrl {
    UIOBJECT_CLASS_NAME_DECLARE(CSkinListCtrl)
public:
    enum {
        TYPE_TEXT_EX                = CColHeader::TYPE_NEXT + 1
    };

    class ItemStringEx : public CSkinListCtrl::ItemString {
    public:
        struct TextColor {
            TextColor(uint8_t _nCountOfText, uint8_t _nClrIndex) : nCountOfText(_nCountOfText), nClrIndex(_nClrIndex) { }
            uint8_t                     nCountOfText;
            uint8_t                     nClrIndex;
        };
        class VecTextColor : public vector<TextColor> {
        public:
            void add(uint8_t _nCountOfText, uint8_t _nClrIndex) {
                push_back(TextColor(_nCountOfText, _nClrIndex));
            }

        };

        VecTextColor                vTextColor;

    };

public:
    virtual Item *newItem(int col) override;

    virtual void addColumn(cstr_t szCol, int nWidth);

    virtual bool setItemText(int nItem, int nSubItem, cstr_t lpszText, bool bRedraw = true) override;
    virtual int insertItem(int nItem, cstr_t lpszItem, int nImageIndex = 0, uint32_t nItemData = 0, bool bRedraw = true) override;

    virtual void drawCell(int row, int col, CRect &rcCell, CRawGraph *canvas, CColor &clrText) override;

    bool setItemTextEx(int nItem, int nSubItem, cstr_t lpszText, const ItemStringEx::VecTextColor & vTextColor);
    int insertItem(int nItem, cstr_t lpszItem, const ItemStringEx::VecTextColor & vTextColor, int nImageIndex = 0, uint32_t nItemData = 0, bool bRedraw = false);


    void drawCellTextEx(ItemStringEx *item, CRect &rcItem, CRawGraph *canvas, CColor &clrText);

};

#endif // !defined(__SKINLISTCTRLEX__H__)
