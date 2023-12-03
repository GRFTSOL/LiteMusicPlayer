#include "SncParser.h"
#include "HelperFun.h"


// COMMENTS:
//    保存为*.snc文件
//    snc 文件的格式：
//        ⑩00003210⑿
//        我躲在车里
int CSncParser::saveAsFile(cstr_t file, const RawLyrics &lyrics) {
    string strData;

    int offsetTime = lyrics.properties().getOffsetTime();
    LyricsLines lines = lyrics.toLyricsLinesOnly();
    for (auto &line : lines) {
        lyricsLineToText(line, offsetTime, strData);
    }

    if (!writeFile(file, strData)) {
        return ERR_WRITE_FILE;
    }

    return ERR_OK;
}

//    snc 文件的格式：
//        ⑩00010450⑿
//          MMSS
//        我躲在车里
bool CSncParser::lyricsLineToText(LyricsLine &line, int offsetTime, string &strBuff) {
    if (!line.isLyricsLine) {
        return false;
    }

    if (line.isTempLine) {
        return false;
    }

    int nMs = line.beginTime + offsetTime;
    int n10Ms = (nMs / 10) % 100;
    int nSec = (nMs / 1000) % 60;
    int nMin = nMs / (1000 * 60);

    strBuff = stringPrintf("⑩%04d%02d%02d⑿\r\n", nMin, nSec, n10Ms).c_str();

    string strLyrics;
    for (auto &piece : line.pieces) {
        strLyrics += piece.text;
    }
    trimStr(strLyrics);

    // 如果本行歌词为空，则不保存本行
    if (strLyrics.empty()) {
        strBuff.clear();
    } else {
        strBuff += strLyrics;
        strBuff += "\r\n";
    }

    return true;
}
