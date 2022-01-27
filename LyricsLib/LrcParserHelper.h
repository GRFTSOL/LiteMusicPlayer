
#pragma once


CHAR_ENCODING getLyricsTextEncoding(uint8_t *&szLrc, int &nLen);


inline bool parseLrcTimeTag(const char *szLine, int &nNextPos, int &nTime)
{
    const char *szTag = szLine;

    nTime = 0;

    if (*szTag != '[')
        goto S_ERROR;

    szTag++;
    // time tag ?
    // seek to ':'
    while (isWhiteSpace(*szTag))
        szTag ++;

    nTime += atoi(szTag) * 60 * 1000;    // nTime += atoi(szTag) * 60s * 1000ms
    if (*szTag == '-')
        szTag++;
    while (isDigit(*szTag))
        szTag ++;
    while (isWhiteSpace(*szTag))
        szTag ++;

    // ':'
    if (*szTag != ':' && *szTag != '.')
        goto S_ERROR;
    szTag ++;

    // number
    while (isWhiteSpace(*szTag))
        szTag ++;

    nTime += atoi(szTag) * 1000;    // nTime += atoi(szTag) * 1000ms
    if (*szTag == '-')
        szTag++;
    while (isDigit(*szTag))
        szTag ++;
    while (isWhiteSpace(*szTag))
        szTag ++;

    if (*szTag == '.' || *szTag == ':')
    {
        szTag ++;
        // new version of lrc file
        while (isWhiteSpace(*szTag))
            szTag ++;

        if (isDigit(szTag[1]))
        {
            // .000 or // .00
            if (isDigit(szTag[2]))
                nTime += atoi(szTag);
            else
                nTime += atoi(szTag) * 10;
        }
        else
            nTime += atoi(szTag) * 100;    // .0
        if (*szTag == '-')
            szTag++;

        while (isDigit(*szTag))
            szTag ++;
        while (isWhiteSpace(*szTag))
            szTag ++;
    }
    if (*szTag != ']')
        goto S_ERROR;
    szTag++;

    nNextPos = (int)(szTag - szLine);

    return true;

S_ERROR:
    return false;
}

inline bool isLrcTimeTag(const char *str)
{
    const char *szTag = str;

    if (*szTag != '[')
        goto S_ERROR;
    szTag++;

    // seek to ':'
    while (isWhiteSpace(*szTag))
        szTag ++;

    if (*szTag == '-')
        szTag++;
    while (isDigit(*szTag))
        szTag ++;
    while (isWhiteSpace(*szTag))
        szTag ++;

    // ':'
    if (*szTag != ':' && *szTag != '.')
        goto S_ERROR;
    szTag ++;

    // number
    while (isWhiteSpace(*szTag))
        szTag ++;

    if (*szTag == '-')
        szTag++;
    while (isDigit(*szTag))
        szTag ++;
    while (isWhiteSpace(*szTag))
        szTag ++;

    if (*szTag == '.' || *szTag == ':')
    {
        szTag ++;
        // new version of lrc file
        while (isWhiteSpace(*szTag))
            szTag ++;

        if (*szTag == '-')
            szTag++;

        while (isDigit(*szTag))
            szTag ++;
        while (isWhiteSpace(*szTag))
            szTag ++;
    }
    if (*szTag != ']')
        goto S_ERROR;
    szTag++;

    return true;

S_ERROR:
    return false;
}

inline bool parseLrcPropTag(const char *szLine, int nLineLen, const string &strPropName,
                     int &nValPosBeg, int &nValPosEnd)
{
    // strPropName Can be empty, return false.
    assert(strPropName.size());

    assert(*szLine == '[');
    const char *p = szLine;
    p++;

    // Ignore spaces
    while (isWhiteSpace(*p))
        p++;

    // Compare name
    if (strncasecmp(p, strPropName.c_str(), strPropName.size()) != 0)
        return false;
    p += strPropName.size();

    nValPosBeg = int(p - szLine);

    // find end position
    const char *pEnd = szLine + nLineLen;
    while (pEnd >= p && *pEnd != ']')
        pEnd--;
    if (pEnd < p)
        nValPosEnd = nLineLen;
    else
        nValPosEnd = pEnd - szLine;

    return true;
}

inline bool parseTxtPropTag(const char *szLine, int nLineLen, const string &strPropName,
                     int &nValPosBeg, int &nValPosEnd)
{
    // strPropName Can be empty, return false.
    if (strPropName.empty())
        return false;

    const char *p = szLine;

    // Compare name
    if (strncasecmp(p, strPropName.c_str(), strPropName.size()) != 0)
        return false;
    p += strPropName.size();

    nValPosBeg = int(p - szLine);
    nValPosEnd = nLineLen;
    assert(nValPosEnd >= nValPosBeg);

    while (nValPosEnd > nValPosBeg)
    {
        char c = szLine[nValPosEnd - 1];
        if (!isWhiteSpace(c) && c != '\r' && c != '\n')
            break;
        nValPosEnd--;
    }

    return true;
}

inline const char * readLine(const char *szText, string &strLine)
{
    assert(szText);
    // search until \n or \0
    const char *p = szText;
    while (*p && *p != '\n' && *p != '\r')
        p++;

    if (*p == '\r' && p[1] == '\n')
        p++;

    // Put in line, and trim \r
    strLine.clear();
    strLine.append(szText, (int)(p - szText));
    while (strLine.size() && strLine[strLine.size() - 1] == '\r')
        strLine.resize(strLine.size() - 1);

    if (*p != '\n' && *p != '\r')
        return nullptr;

    return ++p;
}

inline const char *nextLine(const char *szLine)
{
    while (*szLine && *szLine != '\n' && *szLine != '\r')
        szLine++;

    if (*szLine == '\r' && szLine[1] == '\n')
        szLine++;

    if (*szLine == '\n' || *szLine == '\r')
    {
        szLine++;
        return szLine;
    }

    return nullptr;
}

inline const char *ignoreSpaces(const char *szLine)
{
    while (isWhiteSpace(*szLine))
        szLine++;

    return szLine;
}

inline bool searchLrcPropTag(const char *szText, const string &strPropName,
                      int &nValPosBeg, int &nValPosEnd)
{
    const char *szLine = szText;

    while (szLine)
    {
        const char *szNextLine = nextLine(szLine);

        szLine = ignoreSpaces(szLine);
        if (*szLine == '[' && !isDigit(szLine[1]))
        {
            int nLineLen;
            if (szNextLine)
                nLineLen = int(szNextLine - szLine);
            else
                nLineLen = strlen(szLine);
            if (parseLrcPropTag(szLine, nLineLen, strPropName, nValPosBeg, nValPosEnd))
            {
                // Got it.
                int    nOffset = int(szLine - szText);
                nValPosBeg += nOffset;
                nValPosEnd += nOffset;
                return true;
            }
        }
        szLine = szNextLine;
    }

    return false;
}

inline bool searchTxtPropTag(const char *szText, const string &strPropName,
                      int &nValPosBeg, int &nValPosEnd)
{
    const char *szLine = szText;

    while (szLine)
    {
        const char * szNextLine = nextLine(szLine);

        szLine = ignoreSpaces(szLine);

        int nLineLen;
        if (szNextLine)
            nLineLen = int(szNextLine - szLine);
        else
            nLineLen = strlen(szLine);
        if (parseTxtPropTag(szLine, nLineLen, strPropName, nValPosBeg, nValPosEnd))
        {
            // Got it.
            int    nOffset = int(szLine - szText);
            nValPosBeg += nOffset;
            nValPosEnd += nOffset;
            return true;
        }

        szLine = szNextLine;
    }

    return false;
}
