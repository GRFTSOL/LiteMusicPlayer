#ifndef _FONT_INFO_INC_
#define _FONT_INFO_INC_

#pragma once

#include "../Utils/Utils.h"


#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET     1
#endif

// Font slant
enum {
    FS_NORMAL                   = 0,
    FS_OBLIQUE,
    FS_ITALIC,
};

#ifndef WIN32

#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

#endif

class CFontInfo {
public:
    CFontInfo() {
        m_nSize = 0;
        m_nFontHeight = 0;
        m_weight = FW_NORMAL;
        m_nItalic = 0;
        m_bUnderline = false;
    }
    virtual ~CFontInfo() {
    }

public:
    virtual bool create(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nSize, int nWeight, int nItalic, bool bUnderline = false) {
        m_strNameLatin9 = szFaceNameLatin9;
        if (!szFaceNameOthers || isEmptyString(szFaceNameOthers)) {
            m_strNameOthers = szFaceNameLatin9;
        } else {
            m_strNameOthers = szFaceNameOthers;
        }

        m_nFontHeight = m_nSize = nSize;
        m_weight = nWeight;
        m_nItalic = nItalic;
        m_bUnderline = bUnderline;

        return true;
    }

    virtual bool create(const CFontInfo &font) {
        m_strNameLatin9 = font.m_strNameLatin9;
        m_strNameOthers = font.m_strNameOthers;
        m_nFontHeight = m_nSize = font.m_nSize;
        m_weight = font.m_weight;
        m_nItalic = font.m_nItalic;
        m_bUnderline = font.m_bUnderline;

        return true;
    }

    bool isSame(cstr_t szFaceNameLatin9, cstr_t szFaceNameOthers, int nSize, int nWeight, int nItalic, bool bUnderline) {
        if (strcmp(szFaceNameLatin9, m_strNameLatin9.c_str()) != 0) {
            return false;
        }

        if (!szFaceNameOthers) {
            szFaceNameOthers = szFaceNameLatin9;
        }
        if (strcmp(szFaceNameOthers, m_strNameOthers.c_str()) != 0) {
            return false;
        }

        return nSize == m_nSize && nWeight == m_weight && m_nItalic == nItalic && bUnderline == m_bUnderline;
    }

    bool isSame(const CFontInfo &font) {
        return font.m_strNameLatin9 == m_strNameLatin9
            && font.m_strNameOthers == m_strNameOthers
            && font.m_nSize == m_nSize
            && font.m_weight == m_weight
            && font.m_nItalic == m_nItalic
            && font.m_bUnderline == m_bUnderline;
    }

    bool isValid() { return !m_strNameLatin9.empty(); }

    cstr_t getName() const { return m_strNameLatin9.c_str(); }
    cstr_t getNameOthers() const { return m_strNameOthers.c_str(); }
    int getSize() const { return m_nSize; }
    int getWeight() const { return m_weight; }
    int getItalic() const { return m_nItalic; }
    bool isUnderline() const { return m_bUnderline; }

    virtual int getHeight() const { return m_nFontHeight; }

public:
    string                      m_strNameLatin9, m_strNameOthers;
    int                         m_nSize;
    int                         m_nFontHeight;
    int                         m_weight;
    uint8_t                     m_nItalic;
    bool                        m_bUnderline;

};

#endif // _FONT_INFO_INC_
