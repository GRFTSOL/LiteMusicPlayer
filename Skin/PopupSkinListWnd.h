
#pragma once

#include "PopupSkinWnd.h"
#include "SkinListCtrl.h"

class CPopupSkinListWnd : public CPopupSkinWnd
{
public:
    CPopupSkinListWnd();
    ~CPopupSkinListWnd();

    void onLoadWndSizePos();

    void onSkinLoaded();

    virtual void closeSkin();

    virtual void onUIObjNotify(IUIObjNotify *pNotify);

    class CDataSkinListCtrl : public CSkinListCtrl
    {
    public:
        virtual void invalidate() { }

    };

public:
    void addColumn(cstr_t szCol, int nWidth)
        { m_listCtrlData.addColumn(szCol, nWidth); }
    void addImageColumn(cstr_t szCol, int nWidth)
        { m_listCtrlData.addImageColumn(szCol, nWidth); }

    int getLineHeight() { return m_nLineHeight; }
    void setLineHeight(int nLineHeight)
        { m_nLineHeight = nLineHeight; }

    int getItemCount() const;

    virtual int getCurSel();
    void setCurSel(int nIndex);

    bool deleteItem(int nItem);
    bool deleteAllItems();

    int insertItem(int nItem, cstr_t lpszItem, int nImageIndex = 0, uint32_t nItemData = 0);
    bool setItemText(int nItem, int nSubItem, cstr_t lpszText);
    bool setItemImage(int nItem, int nSubItem, RawImageData *image);

    void setItemData(int nItem, uint32_t nItemData);
    uint32_t getItemData(int nItem);

    string getItemText(int nItem, int nSubItem) const;

protected:
    CDataSkinListCtrl            m_listCtrlData;
    int                            m_nLineHeight;

    CSkinListCtrl                *m_pListCtrl;

    int                            m_nCurSel;

};
