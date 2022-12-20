// DlgChooseFont.cpp: implementation of the CDlgChooseFont class.
//
//////////////////////////////////////////////////////////////////////

#include "../MPlayerApp.h"
#include "DlgChooseFont.h"

// #define FLAG_BOLD        1
// #define FLAG_ITALIC        (0x1 << 1)
// 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgChooseFont::CDlgChooseFont()
{
    m_window = nullptr;
}

CDlgChooseFont::~CDlgChooseFont()
{
    if (m_window)
        gtk_widget_destroy(m_window);
}

int CDlgChooseFont::doModal(Window *pWndParent, cstr_t szFontFaceName, int nFontSize, int nWeight, int nItalic)
{
    int        nRet;

    m_strFontFaceName = szFontFaceName;
    m_nFontSize = nFontSize;
    m_weight = nWeight;
    m_nItalic = nItalic;

    m_window = gtk_font_selection_dialog_new("Select Font");

    {
        // set font name
        string        strFontName;

        // bold
        strFontName += szFontFaceName;
        if (nWeight >= FW_BOLD)
            strFontName += " Bold";
        else if (nWeight >= FW_SEMIBOLD)
            strFontName += " Semi-Bold";

        // italic
        if (nItalic == FS_ITALIC)
            strFontName += " Italic";
        else if (nItalic == FS_OBLIQUE)
            strFontName += " Oblique";

        // font size
        strFontName += " ";
        strFontName += stringPrintf("%d", nFontSize).c_str();

        gtk_font_selection_dialog_set_font_name(GTK_FONT_SELECTION_DIALOG(m_window), strFontName.c_str());
    }

    nRet = gtk_dialog_run(GTK_DIALOG(m_window));

    if (nRet == IDOK)
    {
        bool    bUnderLine;
        gchar    *fontName;
        gchar    *szPos, *szEnd;
        VecStrings    vStr;
//     char *deco[] = {
//         "Semi-Bold", "Bold", 
//         "Italic", 
//         "Semi-Expanded", "Expanded", 
//         "Extra-Condensed", "Semi-Condensed", "Condensed", 
//         "Ultra-Light", "Light", 
//         "Oblique", nullptr };

        fontName = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(m_window));
        StrBreak(fontName, ' ', vStr);

        // font size.
        m_nFontSize = 12;
        if (vStr.size() > 0)
        {
            m_nFontSize = atoi(vStr.back().c_str());
            vStr.pop_back();
        }

        // Italic?
        // Oblique
        m_nItalic = FS_NORMAL;
        if (vStr.size() > 0)
        {
            if (strcasecmp(vStr.back().c_str(), "Italic") == 0)
            {
                m_nItalic = FS_ITALIC;
                vStr.pop_back();
            }
            else if (strcasecmp(vStr.back().c_str(), "Oblique") == 0)
            {
                m_nItalic = FS_OBLIQUE;
                vStr.pop_back();
            }
        }

        // Semi-Bold
        // Bold?
        m_weight = FW_NORMAL;
        if (vStr.size() > 0)
        {
            if (strcasecmp(vStr.back().c_str(), "Bold") == 0)
            {
                m_weight = FW_BOLD;
                vStr.pop_back();
            }
            else if (strcasecmp(vStr.back().c_str(), "Semi-Bold") == 0)
            {
                m_weight = FW_SEMIBOLD;
                vStr.pop_back();
            }
        }

        // "Extra-Condensed", "Semi-Condensed", "Condensed", 
        if (vStr.size() > 0)
        {
            if (strcasecmp(vStr.back().c_str(), "Extra-Condensed") == 0)
            {
                vStr.pop_back();
            }
            else if (strcasecmp(vStr.back().c_str(), "Semi-Condensed") == 0)
            {
                vStr.pop_back();
            }
            else if (strcasecmp(vStr.back().c_str(), "Condensed") == 0)
            {
                vStr.pop_back();
            }
        }

        // "Ultra-Light", "Light", 
        if (vStr.size() > 0)
        {
            if (strcasecmp(vStr.back().c_str(), "Ultra-Light") == 0)
            {
                vStr.pop_back();
            }
            else if (strcasecmp(vStr.back().c_str(), "Light") == 0)
            {
                vStr.pop_back();
            }
        }

        // font name
        m_strFontFaceName.resize(0);
        for (int i = 0; i < vStr.size(); i++)
        {
            if (i != 0)
                m_strFontFaceName += " ";
            m_strFontFaceName += vStr[i];
        }
    }

    return nRet;
}

cstr_t CDlgChooseFont::getFaceName()
{
    return m_strFontFaceName.c_str();
}

int CDlgChooseFont::getSize()
{
    return m_nFontSize;
}

int CDlgChooseFont::getWeight()
{
    return m_weight;
}

int CDlgChooseFont::getItalic()
{
    return m_nItalic;
}
