// ThemesFile.h: interface for the CThemesFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_)
#define AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_

#pragma once


enum ThemeItemID
{
    TII_LYR_COLORS,
    TII_LYR_FONT,
    TII_LYR_STYLE,
    TII_LYR_BG_PIC,
    TII_COUNT,
};

struct ThemeItem
{
    ThemeItemID    nTII;
    cstr_t        szName;
    cstr_t        *properties;
    int            nPropertiesCount;
    int            nIDCtrl;
    bool        bValid;
};

extern ThemeItem    g_themeItems[];
extern const int    g_themeItemsCount;

class CThemesXML : public CSimpleXML
{
public:
    CThemesXML();

    bool remove(cstr_t szThemeName);
    bool setCurTheme(cstr_t szThemeName);
    bool useCurSettingAsTheme(cstr_t szThemeName);

    bool isThemeExists(cstr_t szThemeName);

    void enumAllThemes(vector<string> &vThemes);

protected:
    SXNode::iterator getTheme(cstr_t szThemeName);

    void lyrSettingsToTheme(bool bFloatingLyr, SXNode *pNode);
    void themeToLyrSettings(bool bFloatingLyr, SXNode *pNode);

};


class CThemesFile
{
public:
    CThemesFile()
    {
    }

public:
    bool open();
    bool save();

    bool remove(cstr_t szThemeName);
    bool setCurTheme(cstr_t szThemeName);
    bool useCurSettingAsTheme(cstr_t szThemeName);

    void enumAllThemes(vector<string> &vThemes);

protected:

protected:
    CThemesXML        m_xmlThemes;
    CThemesXML        m_xmlThemesCustomized;

};

#endif // !defined(AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_)
