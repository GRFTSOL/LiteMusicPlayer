// SkinMenu.cpp: implementation of the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinMenu.h"

//////////////////////////////////////////////////////////////////////////

int CSkinMenu::Item::fromXML(CSkinFactory *pSkinFactory, SXNode *pNode)
{
    // <Item Action="append" Type="item" Name="Playlist Editor..." IDCmd="dyncmd_openclose_playlist_wnd" CanCheck="true" IsCheckedCmd="dyncmd_is_mediainfo_bar_visible"/>
    cstr_t        szValue;

    szValue = pNode->getPropertySafe("Type");
    if (strcasecmp(szValue, "item") == 0)
        type = IT_ITEM;
    else if (strcasecmp(szValue, "popup") == 0)
        type = IT_POPUP;
    else if (strcasecmp(szValue, "seperator") == 0)
        type = IT_SEPERATOR;
    else
        type = IT_ITEM;

    szValue = pNode->getPropertySafe("Action");
    if (strcasecmp(szValue, "append") == 0 || strcasecmp(szValue, "InsertByID") == 0)
    {
        if (strcasecmp(szValue, "append") == 0)
            action = A_APPEND;
        else
        {
            action = A_INSERT_BY_ID;
            nPos = pSkinFactory->getIDByName(pNode->getPropertySafe("Position"));
            nPos = pSkinFactory->getMenuIDByUID(nPos, false);
        }

        name = pNode->getPropertySafe("Name");
        bCanCheck = isTRUE(pNode->getPropertySafe("CanCheck"));
        nIDCmd = pSkinFactory->getIDByName(pNode->getPropertySafe("IDCmd"));
        nIDCmdIsChecked = pSkinFactory->getIDByName(pNode->getPropertySafe("IsCheckedCmd"));
        nMenuID = pSkinFactory->getMenuIDByUID(nIDCmd, true);
    }
    else if (strcasecmp(szValue, "remove") == 0)
    {
        action = A_REMOVE;
        nPos = pNode->getPropertyInt("Position");
    }
    else
        action = A_UNKNOWN;

    return ERR_OK;
}

CSkinMenu::CSkinMenu()
{
    m_nOrgAppenPos = 0;
    m_nAppendPosition = 0;
}

CSkinMenu::~CSkinMenu()
{

}

int CSkinMenu::loadMenu(int nID)
{
    string strFile = CSkinApp::getInstance()->getSkinFactory()->getResourceMgr()->getResourcePathName("menu.txt");
    if (strFile.empty())
        return ERR_NOT_FOUND;

    CTextFile file;

    int nRet = file.open(strFile.c_str(), true, ED_LATIN9_ISO);
    if (nRet != ERR_OK)
        return nRet;

    string line;
    file.readLine(line);
    if (strcmp(line.c_str(), "menufile_v1") != 0)
    {
        ERR_LOG1("Invalid menu file format: %s", strFile.c_str());
        return ERR_BAD_FILE_FORMAT;
    }

    createPopupMenu();

    CStrPrintf strMenu("menu:%d", nID);

    while (file.readLine(line))
    {
        if (strcmp(line.c_str(), strMenu.c_str()) != 0)
            continue;

        loadMenu(file, *this);
        
        onLoadMenu();
        return ERR_OK;
    }
    
    ERR_LOG1("Failed to load menu, NOT exists: %d", nID);

    return ERR_NOT_FOUND;
}

void CSkinMenu::loadMenu(CTextFile &file, CMenu &menu)
{
    CColonSeparatedValues values;
    string line;
    while (file.readLine(line))
    {
        VecStrings vValues;
        values.split(line.c_str(), vValues);
        if (vValues.size() == 0)
            continue;

        if (strcmp(vValues[0].c_str(), "popup") == 0)
        {
            CMenu subMenu = menu.appendSubmenu(_TL(vValues[1].c_str()));
            loadMenu(file, subMenu);
        }
        else if (strcmp(vValues[0].c_str(), "item") == 0)
        {
            if (vValues.size() != 4)
            {
                ERR_LOG1("Invalid menu item line: %s", line.c_str());
                continue;
            }
            int nID = stringToInt(vValues[1].c_str(), -1);
            getShortcutKey(nID, vValues[3]);
            menu.appendItem(nID, 
                _TL(vValues[2].c_str()), vValues[3].c_str());
        }
        else if (strcmp(vValues[0].c_str(), "separator") == 0)
        {
            menu.appendSeperator();
        }
        else if (strcmp(vValues[0].c_str(), "end") == 0)
        {
            return;
        }
        else
            ERR_LOG1("Invalid menu item line: %s", line.c_str());
    }
}

void CSkinMenu::trackPopupMenu(int x, int y, Window *pWnd, CRect *prcNotOverlap)
{
    CSkinFactory *pSkinFactory = CSkinApp::getInstance()->getSkinFactory();
    LIST_ITEMS::iterator    it, itEnd;
    int                        nRet;

    itEnd = m_listItems.end();
    for (it = m_listItems.begin(); it != itEnd; ++it)
    {
        Item    &item = *it;
        if (item.nIDCmdIsChecked != UID_INVALID && item.bCanCheck)
        {
            nRet = pSkinFactory->onDynamicCmd(item.nIDCmdIsChecked, (CSkinWnd *)pWnd);
            checkItem(item.nMenuID, nRet == ERR_OK);
        }
    }

    CMenu::trackPopupMenu(x, y, pWnd, prcNotOverlap);
}

void CSkinMenu::trackPopupSubMenu(int x, int y, int nSubMenu, Window *pWnd, CRect *prcNotOverlap)
{
    CSkinFactory *pSkinFactory = CSkinApp::getInstance()->getSkinFactory();
    LIST_ITEMS::iterator    it, itEnd;
    int                        nRet;

    itEnd = m_listItems.end();
    for (it = m_listItems.begin(); it != itEnd; ++it)
    {
        Item    &item = *it;
        if (item.nIDCmdIsChecked != UID_INVALID && item.bCanCheck)
        {
            nRet = pSkinFactory->onDynamicCmd(item.nIDCmdIsChecked, (CSkinWnd *)pWnd);
            checkItem(item.nMenuID, nRet == ERR_OK);
        }
    }

    CMenu::trackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}

int CSkinMenu::fromXML(SXNode *pNodeMenu, int nAppendPos)
{
    if (!pNodeMenu)
        return ERR_NOT_FOUND;

    assert(strcasecmp(pNodeMenu->name.c_str(), "Menu") == 0);

    if (!isValid())
        createPopupMenu();

    m_nAppendPosition = nAppendPos;

    SXNode::iterator it, itEnd;
    itEnd = pNodeMenu->listChildren.end();
    for (it = pNodeMenu->listChildren.begin(); it != itEnd; ++it)
    {
        SXNode        *pNode = *it;

        // OK, found the control
        if (strcasecmp(pNode->name.c_str(), "Item") == 0)
        {
            Item    item;
            item.fromXML(CSkinApp::getInstance()->getSkinFactory(), pNode);

            m_listItems.push_back(item);
            if (item.action == A_APPEND)
            {
                insertItem(m_nAppendPosition, item.nMenuID,
                    _TL(item.name.c_str()));
                m_nAppendPosition++;
            }
            else if (item.action == A_REMOVE)
            {
                removeItem(item.nPos);
                if (item.nPos < m_nAppendPosition)
                    m_nAppendPosition--;
            }
            else  if (item.action == A_INSERT_BY_ID)
            {
                insertItemByID(item.nPos, item.nMenuID,
                    _TL(item.name.c_str()));
            }
        }
    }

    return ERR_OK;
}

int CSkinMenu::getSubMenuPos(int nSubMenu) const
{
    if (nSubMenu > m_nOrgAppenPos)
        return nSubMenu + m_nAppendPosition - m_nOrgAppenPos;
    else
        return nSubMenu;
}
