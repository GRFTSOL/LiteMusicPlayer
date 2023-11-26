#ifndef Skin_SkinMenu_h
#define Skin_SkinMenu_h

#pragma once

#include "../third-parties/rapidjson/rapidjson/document.h"


class CSkinMenu : public CMenu {
public:
    enum ItemType {
        IT_ITEM,
        IT_POPUP,
        IT_SEPERATOR,
    };

    enum Action {
        A_UNKNOWN,
        A_INSERT_BY_ID,
        A_APPEND,
        A_REMOVE,
    };

    struct Item {
        int                         nMenuID;
        int                         nPos;
        ItemType                    type;
        Action                      action;
        bool                        bCanCheck;
        string                      name;
        int                         nIDCmd;
        int                         nIDCmdIsChecked;

        int fromXML(CSkinFactory *pSkinFactory, SXNode *pNode);
    };

    CSkinMenu();
    virtual ~CSkinMenu();

    int loadMenu(const rapidjson::Value &items);

    virtual void updateMenuStatus(Window *window) override;

    virtual int fromXML(SXNode *pNodeMenu, int nAppendPos);

    virtual bool getShortcutKey(int nMenuID, string &strShortcut) { return false; }

    virtual int getOrgAppendPos() { return m_nOrgAppenPos; }

    virtual void setOrgAppendPos(int nPos) { m_nOrgAppenPos = nPos; }

    int getSubMenuPos(int nSubMenu) const;

protected:
    void loadMenu(const rapidjson::Value &items, CMenu &menu);

protected:
    typedef list<Item>        ListItems;

    int                         m_nOrgAppenPos;
    int                         m_nAppendPosition;
    ListItems                   m_listItems;

};

using SkinMenuPtr = std::shared_ptr<CSkinMenu>;

#endif // !defined(Skin_SkinMenu_h)
