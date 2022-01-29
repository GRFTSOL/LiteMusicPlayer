// SkinMenu.h: interface for the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_)
#define AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_

#pragma once

#include "../third-parties/rapidjson/rapidjson/document.h"


class CSkinMenu : public CMenu  
{
public:
    enum ItemType
    {
        IT_ITEM,
        IT_POPUP,
        IT_SEPERATOR,
    };

    enum Action
    {
        A_UNKNOWN,
        A_INSERT_BY_ID,
        A_APPEND,
        A_REMOVE,
    };

    struct Item
    {
        int                nMenuID;
        int                nPos;
        ItemType        type;
        Action            action;
        bool            bCanCheck;
        string            name;
        int                nIDCmd;
        int                nIDCmdIsChecked;

        int fromXML(CSkinFactory *pSkinFactory, SXNode *pNode);
    };

    CSkinMenu();
    virtual ~CSkinMenu();

    int loadMenu(const rapidjson::Value &items);

    void trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap = nullptr);

    void trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap = nullptr);

    virtual int fromXML(SXNode *pNodeMenu, int nAppendPos);

    virtual bool getShortcutKey(int nMenuID, string &strShortcut) { return false; }

    virtual int getOrgAppendPos() { return m_nOrgAppenPos; }

    virtual void setOrgAppendPos(int nPos) { m_nOrgAppenPos = nPos; }

    int getSubMenuPos(int nSubMenu) const;

protected:
    void loadMenu(const rapidjson::Value &items, CMenu &menu);

protected:
    typedef list<Item>        LIST_ITEMS;

    int                m_nOrgAppenPos;
    int                m_nAppendPosition;
    LIST_ITEMS        m_listItems;

};

#endif // !defined(AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_)
