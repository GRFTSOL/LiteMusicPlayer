#include "HelperFun.h"
#include "LyricsParser.h"
#include "LrcParser.h"
#include "SrtParser.h"


//
//7
//00:00:18,454 --> 00:00:21,172
//对我们而言亲吻就像是开场
//
//8
//00:00:21,657 --> 00:00:26,516
//就像佛洛伊飞船出场前
//你得耐著性子先看完脱口秀


int CSrtParser::saveAsFile(cstr_t file, const RawLyrics &rawLyrics) {
    string strData;
    string strBuff;
    char szTime[256];

    //8
    //00:00:21,657 --> 00:00:26,516
    //就像佛洛伊飞船出场前
    //你得耐著性子先看完脱口秀
    int nBegOld = -1, nEndOld = -1;

    int index = 1;
    LyricsLines lyrics = rawLyrics.toLyricsLinesOnly();
    for (LyricsLine &line : lyrics) {
        if (lyricsLineToText(line, strBuff)) {
            if (line.beginTime == nBegOld && line.endTime == nEndOld) {
                // 时间相同的行
                strData += strBuff;
            } else {
                nBegOld = line.beginTime;
                nEndOld = line.endTime;

                if (index > 1) {
                    strData += "\n";
                }

                int nHour, nMinute, nSec, nMs;
                nMs = line.beginTime % 1000;
                nSec = line.beginTime / 1000;
                nMinute = nSec / 60;
                nSec %= 60;
                nHour = nMinute / 60;
                nMinute %= 60;
                snprintf(szTime, CountOf(szTime), "%d\n%02d:%02d:%02d,%03d --> ", index++, nHour, nMinute, nSec, nMs);
                strData += szTime;

                nMs = line.endTime % 1000;
                nSec = line.endTime / 1000;
                nMinute = nSec / 60;
                nSec %= 60;
                nHour = nMinute / 60;
                nMinute %= 60;
                snprintf(szTime, CountOf(szTime), "%02d:%02d:%02d,%03d\n", nHour, nMinute, nSec, nMs);
                strData += szTime;

                strData += strBuff;
            }
        }
    }

    if (!writeFile(file, strData)) {
        return ERR_WRITE_FILE;
    }

    return ERR_OK;
}

// //00:00:21,657
static cstr_t readTime(cstr_t szBeg, uint32_t &dwTime) {
    int n;

    while (*szBeg == ' ') {
        szBeg++;
    }
    szBeg = readInt_t(szBeg, n);
    if (*szBeg != ':') {
        return nullptr;
    }
    szBeg++;
    dwTime = n;

    while (*szBeg == ' ') {
        szBeg++;
    }
    szBeg = readInt_t(szBeg, n);
    if (*szBeg != ':') {
        return nullptr;
    }
    szBeg++;
    dwTime = dwTime * 60 + n;

    while (*szBeg == ' ') {
        szBeg++;
    }
    szBeg = readInt_t(szBeg, n);
    if (*szBeg != ',') {
        return nullptr;
    }
    szBeg++;
    dwTime = dwTime * 60 + n;

    while (*szBeg == ' ') {
        szBeg++;
    }
    szBeg = readInt_t(szBeg, n);
    dwTime = dwTime * 1000 + n;

    return szBeg;
}

// 分析歌词文件，并且存储到 pMLData 中
int CSrtParser::parseFile(cstr_t fileName, bool bUseSpecifiedEncoding, CharEncodingType encoding, RawLyrics &lyricsOut) {
    //8
    //00:00:21,657 --> 00:00:26,516
    //就像佛洛伊飞船出场前
    //你得耐著性子先看完脱口秀
    CTextFile file;

    enum Parse_State {
        PS_NUMBER,
        PS_TIME,
        PS_TXT,
        PS_NEWLINE,
    };
    Parse_State state;
    uint32_t dwBegTime, dwEndTime;

    int nRet = file.open(fileName, true, encoding);
    if (nRet != ERR_OK) {
        return nRet;
    }

    state = PS_NUMBER;
    string line;
    while (file.readLine(line)) {
        trimStr(line, "\r\n");

        cstr_t szBeg = line.c_str();
        if (state == PS_NUMBER) {
            while (isDigit(*szBeg)) {
                szBeg++;
            }
            if (*szBeg != '\0') {
                continue;
            }

            state = PS_TIME;
            continue;
        } else if (state == PS_TIME) {
            szBeg = readTime(szBeg, dwBegTime);
            if (szBeg == nullptr) {
                continue;
            }

            if (strncmp(szBeg, " --> ", 5) != 0) {
                continue;
            }
            szBeg += 5;

            szBeg = readTime(szBeg, dwEndTime);
            if (szBeg == nullptr) {
                continue;
            }

            if (*szBeg != '\0') {
                continue;
            }

            state = PS_TXT;
            continue;
        } else if (state == PS_TXT) {
            if (line.empty()) {
                state = PS_NUMBER;
                continue;
            }

            LyricsLine lyrLine(dwBegTime, dwEndTime);
            lyrLine.appendPiece(dwBegTime, dwEndTime, line);
            lyricsOut.push_back(lyrLine);
        }
    }

    return ERR_OK;
}

bool CSrtParser::lyricsLineToText(const LyricsLine &line, string &strBuff) {
    strBuff.clear();

    if (!line.isLyricsLine || line.isTempLine) {
        return false;
    }

    strBuff = line.joinPiecesText();
    strBuff += "\r\n";

    return true;
}
