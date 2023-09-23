#pragma once


inline bool parseLrcTimeTag(const char *szLine, int &nNextPos, int &nTime) {
    const char *szTag = szLine + nNextPos;

    nTime = 0;

    if (*szTag != '[') {
        goto S_ERROR;
    }

    szTag++;
    // time tag ?
    // seek to ':'
    while (isWhiteSpace(*szTag)) {
        szTag++;
    }

    nTime += atoi(szTag) * 60 * 1000; // nTime += atoi(szTag) * 60s * 1000ms
    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag++;
    }

    // ':'
    if (*szTag != ':' && *szTag != '.') {
        goto S_ERROR;
    }
    szTag++;

    // number
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    nTime += atoi(szTag) * 1000; // nTime += atoi(szTag) * 1000ms
    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag ++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    if (*szTag == '.' || *szTag == ':') {
        szTag ++;
        // new version of lrc file
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }

        if (isDigit(szTag[1])) {
            // .000 or // .00
            if (isDigit(szTag[2])) {
                nTime += atoi(szTag);
            } else {
                nTime += atoi(szTag) * 10;
            }
        } else {
            nTime += atoi(szTag) * 100; // .0
        }
        if (*szTag == '-') {
            szTag++;
        }

        while (isDigit(*szTag)) {
            szTag++;
        }
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }
    }
    if (*szTag != ']') {
        goto S_ERROR;
    }
    szTag++;

    nNextPos = (int)(szTag - szLine);

    return true;

S_ERROR:
    return false;
}

inline bool isLrcTimeTag(const char *str) {
    const char *szTag = str;

    if (*szTag != '[') {
        goto S_ERROR;
    }
    szTag++;

    // seek to ':'
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag ++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    // ':'
    if (*szTag != ':' && *szTag != '.') {
        goto S_ERROR;
    }
    szTag ++;

    // number
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    if (*szTag == '-') {
        szTag++;
    }
    while (isDigit(*szTag)) {
        szTag ++;
    }
    while (isWhiteSpace(*szTag)) {
        szTag ++;
    }

    if (*szTag == '.' || *szTag == ':') {
        szTag ++;
        // new version of lrc file
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }

        if (*szTag == '-') {
            szTag++;
        }

        while (isDigit(*szTag)) {
            szTag ++;
        }
        while (isWhiteSpace(*szTag)) {
            szTag ++;
        }
    }
    if (*szTag != ']') {
        goto S_ERROR;
    }
    szTag++;

    return true;

S_ERROR:
    return false;
}
