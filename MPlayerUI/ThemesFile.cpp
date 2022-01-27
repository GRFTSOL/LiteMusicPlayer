// ThemesFile.cpp: implementation of the CThemesFile class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "ThemesFile.h"
#include "Helper.h"

cstr_t        __vLyrClrProperties[] = {
    "Ed_HighColor",
    "Ed_LowColor",
    "Ed_TagColor",
    "Ed_BgColor",
    "Ed_FocusLineBgColor",
    "BgColor",
    "OutlineLyrText",

    "HilightOBM",
    "FgColor",
    "HilightGradient1", "HilightGradient2", "HilightGradient3",
    "HilightBorderColor",
    "HilightPattern",

    "LowlightOBM",
    "FgLowColor",
    "LowlightGradient1", "LowlightGradient2", "LowlightGradient3",
    "LowlightBorderColor",
    "LowlightPattern",
};

cstr_t        __LyrFontProperties[] = {
    "Font",
    "LineSpacing",
};

cstr_t        __LyrStyleProperties[] = {
    "LyrAlign",
    "LyrDrawOpt",
    "Karaoke",
    "LyrDisplayStyle",
};

cstr_t        __LyrBgPicProperties[] = {
    "UseBgImage",
    "BgPicFolder",
    "UseAlbumArtAsBg",
    "DarkenLyrBg",
    "SlideDelayTime",
};

ThemeItem    g_themeItems[] = 
{
    {
        TII_LYR_COLORS,
        "LyrColors",
        __vLyrClrProperties,
        CountOf(__vLyrClrProperties),
        IDC_C_LYR_COLORS,
        true,
    },
    {
        TII_LYR_FONT,
        "LyrFont",
        __LyrFontProperties,
        CountOf(__LyrFontProperties),
        IDC_C_LYR_FONT,
        true,
    },
    {
        TII_LYR_STYLE,
        "LyrStyle",
        __LyrStyleProperties,
        CountOf(__LyrStyleProperties),
        IDC_C_LYR_STYLE,
        true,
    },
    {
        TII_LYR_BG_PIC,
        "LyrBgPic",
        __LyrBgPicProperties,
        CountOf(__LyrBgPicProperties),
        IDC_C_LYR_BG_PIC,
        false,
    },
};

const int    g_themeItemsCount = CountOf(g_themeItems);


#define SZ_THEMES_FILE        "Themes.xml"
#define SZ_CUST_THEMES_FILE    "ThemesCust.xml"
#define SZ_NAME                "Name"
#define SZ_THEMES            "Themes"

CThemesXML::CThemesXML()
{
    New(SZ_THEMES);
}

bool CThemesXML::remove(cstr_t szThemeName)
{
    if (!m_pRoot)
        return false;

    SXNode::iterator    it = getTheme(szThemeName);
    if (it == m_pRoot->listChildren.end())
        return false;

    SXNode        *pNode = *it;
    m_pRoot->listChildren.erase(it);
    delete pNode;

    return true;
}

bool CThemesXML::setCurTheme(cstr_t szThemeName)
{
    if (!m_pRoot)
        return false;

    SXNode::iterator    it = getTheme(szThemeName);
    if (it == m_pRoot->listChildren.end())
        return false;

    SXNode        *pNode = *it;

    SXNode        *pNodeLyr = pNode->getChild("NormalLyr");
    if (pNodeLyr)
        themeToLyrSettings(false, pNodeLyr);

    SXNode        *pNodeFloatingLyr = pNode->getChild("FloatingLyr");
    if (pNodeFloatingLyr)
        themeToLyrSettings(true, pNodeFloatingLyr);

    return true;
}

bool CThemesXML::useCurSettingAsTheme(cstr_t szThemeName)
{
    if (!m_pRoot)
        New(SZ_THEMES);

    remove(szThemeName);

    SXNode        *pNode = new SXNode;

    pNode->name = "Theme";
    pNode->addProperty(SZ_NAME, szThemeName);

    SXNode        *pNodeLyr = new SXNode;
    pNodeLyr->name = "NormalLyr";
    lyrSettingsToTheme(false, pNodeLyr);
    pNode->listChildren.push_back(pNodeLyr);

    SXNode        *pNodeFloatingLyr = new SXNode;
    pNodeFloatingLyr->name = "FloatingLyr";
    lyrSettingsToTheme(true, pNodeFloatingLyr);
    pNode->listChildren.push_back(pNodeFloatingLyr);

    m_pRoot->listChildren.push_back(pNode);

    return true;
}

bool CThemesXML::isThemeExists(cstr_t szThemeName)
{
    if (!m_pRoot)
        return false;

    return getTheme(szThemeName) != m_pRoot->listChildren.end();
}

void CThemesXML::enumAllThemes(vector<string> &vThemes)
{
    if (!m_pRoot)
        return;

    for (SXNode::iterator it = m_pRoot->listChildren.begin(); it != m_pRoot->listChildren.end(); ++it)
    {
        SXNode        *pNode = *it;
        cstr_t        szName = pNode->getProperty(SZ_NAME);
        if (szName)
            vThemes.push_back(szName);
    }
}

SXNode::iterator CThemesXML::getTheme(cstr_t szThemeName)
{
    assert(m_pRoot);

    for (SXNode::iterator it = m_pRoot->listChildren.begin(); it != m_pRoot->listChildren.end(); ++it)
    {
        SXNode        *pNode = *it;
        if (strcasecmp(pNode->getPropertySafe(SZ_NAME), szThemeName) == 0)
            return it;
    }

    return m_pRoot->listChildren.end();
}

static void addSubPropNode(SXNode *pNodeParent, cstr_t szSect, cstr_t szSubPropName, cstr_t szNames[], int nCount)
{
    cstr_t        szContent;
    SXNode        *pNodeChild = new SXNode;
    pNodeChild->name = szSubPropName;

    for (int i = 0; i < nCount; i++)
    {
        szContent = g_profile.getString(szSect, szNames[i], "");
        if (!szContent || isEmptyString(szContent))
            continue;

        SXNode        *pNode = new SXNode;

        pNode->name = szNames[i];
        pNode->strContent = szContent;

        pNodeChild->listChildren.push_back(pNode);
    }

    pNodeParent->listChildren.push_back(pNodeChild);
}

void CThemesXML::lyrSettingsToTheme(bool bFloatingLyr, SXNode *pNode)
{
    assert(m_pRoot);

    string strSectName;
    EventType etDispSettings;
    CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr, strSectName, etDispSettings);

    for (int i = 0; i < g_themeItemsCount; i++)
    {
        addSubPropNode(pNode, strSectName.c_str(), g_themeItems[i].szName, 
            g_themeItems[i].properties, g_themeItems[i].nPropertiesCount);
    }
}

void CThemesXML::themeToLyrSettings(bool bFloatingLyr, SXNode *pNode)
{
    assert(m_pRoot);

    string strSectName;
    EventType etDispSettings;
    CMPlayerApp::getInstance()->getCurLyrDisplaySettingName(bFloatingLyr, strSectName, etDispSettings);

    for (int i = 0; i < g_themeItemsCount; i++)
    {
        if (!g_themeItems[i].bValid)
            continue;

        SXNode *pNodeChild = pNode->getChild(g_themeItems[i].szName);
        if (!pNodeChild)
            continue;

        for (SXNode::iterator it = pNodeChild->listChildren.begin(); it != pNodeChild->listChildren.end(); ++it)
        {
            SXNode    *pProperty = *it;
            CMPlayerSettings::setSettings(etDispSettings, strSectName.c_str(), 
                pProperty->name.c_str(), pProperty->strContent.c_str());
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool CThemesFile::open()
{
    string        strFile;

    strFile = getAppResourceDir();
    strFile += SZ_THEMES_FILE;
    m_xmlThemes.parseFile(strFile.c_str());

    strFile = getAppDataDir();
    strFile += SZ_CUST_THEMES_FILE;
    m_xmlThemesCustomized.parseFile(strFile.c_str());

    return true;
}

bool CThemesFile::save()
{
    string        strFile;

    strFile = getAppResourceDir();
    strFile += SZ_THEMES_FILE;
    m_xmlThemes.saveFile(strFile.c_str());

    strFile = getAppDataDir();
    strFile += SZ_CUST_THEMES_FILE;
    return tobool(m_xmlThemesCustomized.saveFile(strFile.c_str()));
}

bool CThemesFile::remove(cstr_t szThemeName)
{
    m_xmlThemes.remove(szThemeName);
    m_xmlThemesCustomized.remove(szThemeName);

    return true;
}

bool CThemesFile::setCurTheme(cstr_t szThemeName)
{
    if (m_xmlThemesCustomized.setCurTheme(szThemeName))
        return true;

    return m_xmlThemes.setCurTheme(szThemeName);
}

bool CThemesFile::useCurSettingAsTheme(cstr_t szThemeName)
{
    m_xmlThemes.remove(szThemeName);

    return m_xmlThemesCustomized.useCurSettingAsTheme(szThemeName);
}

void CThemesFile::enumAllThemes(vector<string> &vThemes)
{
    m_xmlThemes.enumAllThemes(vThemes);
    m_xmlThemesCustomized.enumAllThemes(vThemes);
    std::sort(vThemes.begin(), vThemes.end());
}
