#include "Player.h"
#include "PlayListFile.h"


void removeReturn(char * str) {
    char * p = str + strlen(str) - 1;

    while (p >= str && (*p == ' ' || *p == '\r' || *p == '\n')) {
        *p = '\0';
        p--;
    }
}

bool loadPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile) {
    assert(player);
    assert(playList);

    if (fileIsExtSame(szFile, ".m3u")) {
        return loadM3uPlaylist(player, playList, szFile);
    } else if (fileIsExtSame(szFile, ".pls")) {
        return loadPlsPlaylist(player, playList, szFile);
    } else if (fileIsExtSame(szFile, ".wpl")) {
        return loadWplPlaylist(player, playList, szFile);
    } else {
        return false;
    }
}

#define EXTINF              "#EXTINF"
#define EXTM3U              "#EXTM3U"
#define WPL_HEADER          "<?wpl version=\"1.0\"?>"
#define XML_HEADER          "<?xml version=\"1.0\" encoding='UTF-8'?>"

// three format of szFileName:
// G:\mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3
// \mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3
// 02.(You Drive Me ) Crazy.MP3
bool appendMediaToList(IMPlayer *player, IPlaylist *playList, cstr_t szFileName, cstr_t szBasePath) {
    MLRESULT nRet;
    CMPAutoPtr<IMedia> media;
    string strFile;

    if (szBasePath[1] == ':') {
        if (szFileName[0] == '\\') {
            strFile.append(szBasePath, 2);
            strFile += szFileName;
        } else if (szFileName[1] == ':') {
            strFile = szFileName;
        } else {
            strFile = szBasePath;
            strFile += szFileName;
        }
    } else if (szBasePath[0] == '\\') {
        if (szFileName[0] == '\\') {
            strFile = szFileName;
        } else {
            strFile = szBasePath;
            strFile += szFileName;
        }
    } else {
        strFile = szFileName;
    }

    if (player->newMedia(&media, strFile.c_str()) == ERR_OK) {
        nRet = playList->insertItem(-1, media);
        if (nRet != ERR_OK) {
            ERR_LOG0("playList->insertItem FAILED!");
            return false;
        }
    } else {
        ERR_LOG0("playList->newMedia FAILED!");
        return false;
    }

    return true;
};

bool savePlaylistAsM3u(IPlaylist    *playList, cstr_t szFile) {
    // M3U
    // #EXTM3U
    // #EXTINF:211,Britney Spears - ...Baby One More Time
    // G:\mp3\Britney Spears04\01.Baby One More Time.MP3
    // ...

    auto nCount = playList->getCount();
    auto fp = fopen(szFile, "w");
    if (!fp) {
        return false;
    }

    // header tag
    fprintf(fp, "#EXTM3U\n");

    //
    for (int i = 0; i < nCount; i++) {
        CMPAutoPtr<IMedia> media;
        auto nRet = playList->getItem(i, &media);
        if (nRet == ERR_OK) {
            CXStr strUrl;
            nRet = media->getSourceUrl(&strUrl);
            if (nRet == ERR_OK) {
                int nDuration = media->getDuration();
                string strFullTitle = g_Player.formatMediaTitle(media);

                // #EXTINF:211,Britney Spears - ...Baby One More Time
                // G:\mp3\Britney Spears04\01.Baby One More Time.MP3
                if (nDuration == MEDIA_LENGTH_INVALID) {
                    fprintf(fp, "#EXTINF:%s\n", strFullTitle.c_str());
                } else {
                    fprintf(fp, "#EXTINF:%d,%s\n", nDuration, strFullTitle.c_str());
                }

                fprintf(fp, "%s", strUrl.c_str());
                fprintf(fp, "\n");
            }
        }
    }

    fclose(fp);

    return true;
}

bool loadM3uPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile) {
    char szBuff[MAX_PATH * 2];
    FILE *fp;
    string strFile;
    string str;

    fp = fopen(szFile, "r");
    if (!fp) {
        return false;
    }

    string strPath = fileGetPath(szFile);

    // #EXTM3U
    if (!fgets(szBuff, CountOf(szBuff), fp)) {
        return false;
    }
    if (strncasecmp(szBuff, EXTM3U, strlen(EXTM3U)) != 0) {
        return false;
    }

    while (fgets(szBuff, CountOf(szBuff), fp)) {
        removeReturn(szBuff);
        if (szBuff[0] == '#') {
            //             if (strncmp(szBuff, EXTINF, strlen(EXTINF)) == 0)
            //             {
            //                 char *        p;
            //                 p = strchr(szBuff, ',');
            //                 if (p)
            //                 {
            //                     p++;
            //                     while (*p == ' ')
            //                         p++;
            //                     if (!isEmptyString(p))
            //                     {
            //                         // p is title now
            //                     }
            //                 }
            //             }
            continue;
        }

        if (isEmptyString(szBuff)) {
            continue;
        }

        appendMediaToList(player, playList, szBuff, strPath.c_str());
    }
    fclose(fp);

    return true;
}

bool loadPlsPlaylist(IMPlayer *player, IPlaylist *playList, cstr_t szFile) {
    CProfile file;
    int n;
    char szkey[256];

    string strPath = fileGetPath(szFile);

    file.init(szFile, "playlist");
    n = file.getInt("NumberOfEntries", 0);

    for (int i = 1; i < n; i++) {
        sprintf(szkey, "File%d", i);

        string strFile;
        strFile = file.getString(szkey, "");
        if (strFile.empty()) {
            continue;
        }

        appendMediaToList(player, playList, strFile.c_str(), strPath.c_str());
    }

    return true;
}

bool loadWplPlaylist(IMPlayer *player, IPlaylist    *playList, cstr_t szFile) {
    string buffer;

    string strPath = fileGetPath(szFile);

    if (!readFile(szFile, buffer)) {
        return false;
    }

    if (strncmp(buffer.c_str(), WPL_HEADER, strlen(WPL_HEADER)) != 0) {
        ERR_LOG1("Unknown wmp file format(header): %s", szFile);
        return false;
    }

    buffer.replace(0, strlen(WPL_HEADER), XML_HEADER, strlen(XML_HEADER));

    CSimpleXML xml;

    if (!xml.parseData(buffer.data(), buffer.size())) {
        ERR_LOG1("Unknown wmp file format(parse): %s", szFile);
        return false;
    }
    //     <smil>
    //         <body>
    //             <seq>
    //                 <media src="G:\mp3\Britney Spears04\02.(You Drive Me ) Crazy.MP3"/>
    //             </seq>
    //         </body>
    //     </smil>

    SXNode *pNode;
    pNode = xml.m_pRoot;
    if (strcmp(pNode->name.c_str(), "smil") != 0) {
        ERR_LOG1("Unknown wmp file format(field: smil): %s", szFile);
        return false;
    }

    pNode = pNode->getChild("body");
    if (!pNode) {
        ERR_LOG1("Unknown wmp file format(field: body): %s", szFile);
        return false;
    }

    pNode = pNode->getChild("seq");
    if (!pNode) {
        ERR_LOG1("Unknown wmp file format(field: seq): %s", szFile);
        return false;
    }

    SXNode::iterator it = pNode->listChildren.begin();
    for (; it != pNode->listChildren.end(); ++it) {
        SXNode *pNodeMedia = *it;
        if (strcmp(pNodeMedia->name.c_str(), "media") == 0) {
            cstr_t szFile = pNodeMedia->getPropertySafe("src");
            if (szFile && !isEmptyString(szFile)) {
                appendMediaToList(player, playList, szFile, strPath.c_str());
            }
        }
    }

    return true;
}
