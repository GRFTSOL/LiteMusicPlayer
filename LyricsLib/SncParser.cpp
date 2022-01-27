// SncParser.cpp: implementation of the CSncParser class.
//
//////////////////////////////////////////////////////////////////////

#include "MLLib.h"
#include "SncParser.h"
#include "HelperFun.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSncParser::CSncParser(CMLData *pMLData) : CLyricsParser(pMLData)
{

}

CSncParser::~CSncParser()
{

}

// COMMENTS:
//    保存为*.snc文件
//    snc 文件的格式：
//        ⑩00003210⑿
//        我躲在车里
int CSncParser::saveAsFile(cstr_t file)
{
    int            nCount;
    string    strData;
    string    strBuff;

    nCount = (int)m_pMLData->m_arrFileLines.size();
    for (int i = 0; i < nCount; i++)
    {
        LyricsLine *pLine;

        pLine = m_pMLData->m_arrFileLines[i];

        if (pLine)
        {
            // save row.
            if (lyricsLineToText(pLine, strBuff))
            {
                strData += strBuff;
            }
        }
    }

    if (!writeFile(file, strData))
        return ERR_WRITE_FILE;

    return ERR_OK;
}

//    snc 文件的格式：
//        ⑩00010450⑿
//          MMSS
//        我躲在车里
bool CSncParser::lyricsLineToText(LyricsLine *pLine, string &strBuff)
{
    strBuff.clear();

    if (!pLine->bLyricsLine)
        return false;

    if (pLine->isTempLine())
        return false;

    int nMs = pLine->nBegTime + m_pMLData->getOffsetTime();
    int n10Ms = (nMs / 10) % 100;
    int nSec = (nMs / 1000) % 60;
    int nMin = nMs / (1000 * 60);

    strBuff = CStrPrintf("⑩%04d%02d%02d⑿\r\n", nMin, nSec, n10Ms).c_str();

    int            nCount;
    string strLyrics;

    nCount = (int)pLine->vFrags.size();
    for (int i = 0; i < nCount; i++)
    {
        strLyrics += pLine->vFrags[i]->szLyric;
    }
    trimStr(strLyrics);

    // 如果本行歌词为空，则不保存本行
    if (strLyrics.empty())
        strBuff.clear();
    else
    {
        strBuff += strLyrics;
        strBuff += "\r\n";
    }

    return true;
}
