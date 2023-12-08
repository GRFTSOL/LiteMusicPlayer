#include "../Utils/UtilsTypes.h"
#include "LyricsKeywordFilter.h"


bool isNumeric(cwstr_t szString);

WCHAR CLyricsKeywordFilter::m_wTableUcs2[WORD_MAX];

void eraseNumb(utf16string &str) {
    WCHAR *szPtr, *szNew;
    szNew = szPtr = (WCHAR *)str.data();
    while (*szPtr) {
        if (!isDigit(*szPtr)) {
            *szNew = *szPtr;
            szNew++;
        }
        szPtr++;
    }
    *szNew = '\0';
    str.resize(wcslen(str.c_str()));
}

void eraseSpace(utf16string &str) {
    WCHAR *szPtr, *szNew;
    szNew = szPtr = (WCHAR *)str.data();
    while (*szPtr) {
        if (!isWhiteSpace(*szPtr)) {
            *szNew = *szPtr;
            szNew++;
        }
        szPtr++;
    }
    *szNew = '\0';
    str.resize(wcslen(str.c_str()));
}

bool isNumeric(cwstr_t szString) {
    cwstr_t szTemp = szString;

    while (isDigit(*szTemp)) {
        szTemp++;
    }

    if (*szTemp != '\0') {
        return false;
    }

    // empty String
    if (szString == szTemp) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////

CLyricsKeywordFilter::CLyricsKeywordFilter() {
}

CLyricsKeywordFilter::~CLyricsKeywordFilter() {

}

void CLyricsKeywordFilter::init(cstr_t fnFilter) {
    for (int i = 0; i < WORD_MAX; i++) {
        m_wTableUcs2[i] = (uint16_t)i;
    }

    CSimpleXML xml;
    if (xml.parseFile(fnFilter)) {
        assert(xml.m_pRoot->name == "KeywordFilter");
        for (auto node : xml.m_pRoot->listChildren) {
            if (node->name == "Remove") {
                // remove
                utf16string strW;

                utf8ToUCS2(node->strContent.c_str(), node->strContent.size(), strW);
                ucs2TableDelChars(strW.c_str());
            } else if (node->name == "Replace") {
                // replace
                utf16string strWSrc, strWDst;

                utf8ToUCS2(node->getPropertySafe("src"), -1, strWSrc);
                utf8ToUCS2(node->getPropertySafe("dst"), -1, strWDst);
                ucs2TableReplaceChars(strWSrc.c_str(), strWDst.c_str());
            }
        }
    } else {
        // ERRLOG1("Failed to parse lyrics keyword filter: %s", fnFilter);
    }

    xTableDelChars(" !\"#$%&'*+,-./:;<=>?@\\^_`|~");
    xTableReplaceChars("[]{}", "()()");
    xTableReplaceChars("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "abcdefghijklmnopqrstuvwxyz");
//
//    TestCLyricsKeywordFilter();
//    TestCLyricsKeywordFilterUtf8();
}

void CLyricsKeywordFilter::xTableDelChars(const char *szChars) {
    utf16string strOut;
    utf8ToUCS2(szChars, -1, strOut);
    ucs2TableDelChars(strOut.c_str());
}

void CLyricsKeywordFilter::xTableReplaceChars(const char *szReplace, const char *szTo) {
    utf16string strReplaceW, strToW;
    utf8ToUCS2(szReplace, -1, strReplaceW);
    utf8ToUCS2(szTo, -1, strToW);
    ucs2TableReplaceChars(strReplaceW.c_str(), strToW.c_str());
}

void CLyricsKeywordFilter::ucs2TableDelChars(const WCHAR *szChars) {
    while (*szChars) {
        m_wTableUcs2[*szChars] = 0;
        szChars++;
    }
}

void CLyricsKeywordFilter::ucs2TableReplaceChars(const WCHAR *szReplace, const WCHAR *szTo) {
    if (wcslen(szTo) == 1) {
        WCHAR ch = szTo[0];
        while (*szReplace) {
            m_wTableUcs2[*szReplace] = ch;
            szReplace++;
        }
        return;
    }

    while (*szReplace && *szTo) {
        m_wTableUcs2[*szReplace] = *szTo;
        szReplace++;
        szTo++;
    }
}

void CLyricsKeywordFilter::filter(const char *szTarg, string &strOut) {
    utf16string strTargUcs2, strOutUcs2;

    utf8ToUCS2(szTarg, -1, strTargUcs2);

    filter(strTargUcs2.c_str(), strOutUcs2);

    ucs2ToUtf8(strOutUcs2.c_str(), (int)strOutUcs2.size(), strOut);
}

void CLyricsKeywordFilter::filter(cwstr_t szInput, utf16string &strOut) {
    if (m_wTableUcs2['a'] == 0) {
        // Isn't initialized?
        assert(m_wTableUcs2['a'] != 0);
    }

    strOut.clear();

    int nCommentDeep = 0;

    cwstr_t p = szInput;
    while (*p) {
        uint16_t wTo;

        wTo = m_wTableUcs2[*p];
        p++;

        // target?
        // comment: between ( and ) is comment.
        if (wTo != 0) {
            if (nCommentDeep == 0) {
                if (wTo == '(') {
                    nCommentDeep++;
                } else if (wTo != ')') {
                    strOut.push_back(wTo);
                }
            } else {
                if (wTo == ')') {
                    nCommentDeep--;
                } else if (wTo == '(') {
                    nCommentDeep++;
                }
            }
        }
    }

    if (!isNumeric(strOut.c_str())) {
        eraseNumb(strOut);
    }

    if (strOut.empty()) {
        strOut = szInput;
        eraseSpace(strOut);
    }
}
