#include "MediaTags.h"

#include "ID3/ID3v1.h"
#include "ID3/ID3v2.h"
#include "../LyricsLib/Lyrics3v2.h"
#include "MidiTag.h"

#include "MP3InfoReader.h"


#define MIN_EMBEDDED_LYR_SIZE    10        // Minum file size of embedded lyrics

IdToString    __id2strLyrSrcType[] = {
    { LST_ID3V2_SYLT, SZ_SONG_ID3V2_SYLT },
    { LST_ID3V2_USLT, SZ_SONG_ID3V2_USLT },
    { LST_LYRICS3V2, SZ_SONG_LYRICS3V2 },
    { LST_ID3V2_LYRICS, SZ_SONG_ID3V2_LYRICS },
    { LST_M4A_LYRICS, SZ_SONG_M4A_LYRICS },
    { LST_KAR_LYRICS, SZ_SONG_KAR_LYRICS },
    { LST_UNKNOWN, nullptr },
};

IdToString    __id2strLyrSrcTypeDesc[] = {
    { LST_ID3V2_SYLT, SZ_SONG_ID3V2_SYLT_DESC },
    { LST_ID3V2_USLT, SZ_SONG_ID3V2_USLT_DESC },
    { LST_LYRICS3V2, SZ_SONG_LYRICS3V2_DESC },
    { LST_ID3V2_LYRICS, SZ_SONG_ID3V2_LYRICS_DESC },
    { LST_M4A_LYRICS, SZ_SONG_M4A_LYRICS_DESC },
    { LST_KAR_LYRICS, SZ_SONG_KAR_LYRICS_DESC },
    { LST_NONE, SZ_SONG_NO_LYRICS_DESC },
    { LST_UNKNOWN, nullptr },
    // { ,  },
};

LRC_SOURCE_TYPE lyrSrcTypeFromName(cstr_t szLrcSource) {
    if (strcasecmp(szLrcSource, NONE_LYRCS) == 0) {
        return LST_NONE;
    }

    for (int i = 0; __id2strLyrSrcType[i].szId != nullptr; i++) {
        if (startsWith(szLrcSource, __id2strLyrSrcType[i].szId)) {
            return (LRC_SOURCE_TYPE)__id2strLyrSrcType[i].dwId;
        }
    }

    return LST_FILE;
}

cstr_t lyrSrcTypeToName(LRC_SOURCE_TYPE lst) {
    if (lst == LST_NONE) {
        return NONE_LYRCS;
    }
    return iDToString(__id2strLyrSrcType, lst, "Unknown");
}

cstr_t lyrSrcTypeToDesc(LRC_SOURCE_TYPE lst) {
    return iDToString(__id2strLyrSrcTypeDesc, lst, "Unknown");
}

bool getEmbeddedLyricsNameInfo(cstr_t szName, string &language, int &index) {
    language.clear();
    index = 0;

    if (szName == nullptr) {
        return true; // No language or index is set.
    }

    szName = szName + strlen("song://");
    VecStrings vStr;

    strSplit(szName, '/', vStr);

    if (vStr.size() < 2) {
        return false;
    }

    //     type = vStr[0];
    //     subType = vStr[1];

    if (vStr.size() >= 3) {
        language = vStr[2];
    }

    if (vStr.size() >= 4) {
        index = atoi(vStr[3].c_str());
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////

MediaTags::MediaTags(void) {
}

MediaTags::~MediaTags(void) {
}

bool isStrInArray(cstr_t szArray[], cstr_t str) {
    for (int i = 0; szArray[i] != nullptr; i++) {
        if (strcasecmp(szArray[i], str) == 0) {
            return true;
        }
    }

    return false;
}

bool MediaTags::isMediaTypeSupported(cstr_t szFile) {
    if (isID3v2TagSupported(szFile)) {
        return true;
    }

    if (isM4aTagSupported(szFile)) {
        return true;
    }

    return false;
}

int MediaTags::getTagFast(cstr_t szFile, BasicMediaTags &tags) {
    FILE *fp = fopen(szFile, "rb");
    if (!fp) {
        setCustomErrorDesc(stringPrintf("%s: %s", OSError().Description(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    if (isID3v2TagSupported(szFile)) {
        CMP3InfoReader reader;
        reader.attach(fp);
        tags.mediaLength = reader.getMediaLength();

        CID3v2IF id3v2(ED_SYSDEF);
        int nRet = id3v2.open(fp, false);
        if (nRet == ERR_OK) {
            id3v2.getTags(tags);
        } else {
            CID3v1 id3v1;
            return id3v1.getTag(fp, tags);
        }
    } else if (isM4aTagSupported(szFile)) {
        CM4aTag tag;
        int nRet = tag.open(fp, false);
        if (nRet != ERR_OK) {
            return nRet;
        }

        return tag.getTags(tags);
    }

    return ERR_OK;
}

int MediaTags::getEmbeddedLyrics(cstr_t szFile, VecStrings &vLyricsNames) {
    if (!isFileExist(szFile)) {
        return ERR_NOT_FOUND;
    }

    if (isID3v2TagSupported(szFile)) {
        // ID3v2 lyrics
        CID3v2IF id3v2(ED_SYSDEF);
        int nRet;

        nRet = id3v2.open(szFile, false, false);
        if (nRet == ERR_OK) {
            VecStrings vLyrics;
            nRet = id3v2.listLyrics(vLyricsNames);
            id3v2.close();
            if (nRet != ERR_OK) {
                return nRet;
            }
        }
    }

    if (isID3v2TagSupported(szFile)) {
        // Lyrics3v2
        CLyrics3v2 lyrics3v2;
        string strLyrics;

        if (lyrics3v2.readLyrcsInfo(szFile, strLyrics) == ERR_OK) {
            if (strLyrics.size() > MIN_EMBEDDED_LYR_SIZE) {
                vLyricsNames.push_back(SZ_SONG_LYRICS3V2);
            }
        }
    }

    if (isM4aTagSupported(szFile)) {
        CM4aTag tag;
        int nRet = tag.open(szFile, false);
        if (nRet != ERR_OK) {
            return nRet;
        }

        nRet = tag.listLyrics(vLyricsNames);
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    if (isKarTagSupported(szFile)) {
        CMidiTag tag;
        int nRet = tag.open(szFile);
        if (nRet != ERR_OK) {
            return nRet;
        }

        nRet = tag.listLyrics(vLyricsNames);
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    return ERR_OK;
}

bool MediaTags::isEmbeddedLyricsSupported(cstr_t szFile) {
    return isID3v2TagSupported(szFile) || isM4aTagSupported(szFile);
}

bool MediaTags::isID3v2TagSupported(cstr_t szFile) {
    return fileIsExtSame(szFile, ".mp3") || fileIsExtSame(szFile, ".mp2");
}

bool MediaTags::isM4aTagSupported(cstr_t szFile) {
    return fileIsExtSame(szFile, ".m4a");
}

bool MediaTags::isKarTagSupported(cstr_t szFile) {
    return fileIsExtSame(szFile, ".kar") || fileIsExtSame(szFile, ".mid");
}

int MediaTags::removeEmbeddedLyrics(cstr_t szMediaFile, VecStrings &vLyrNamesToRemove, int *succeededCount) {
    uint32_t uAllLstFlag = 0;
    for (uint32_t i = 0; i < vLyrNamesToRemove.size(); i++) {
        uAllLstFlag |= lyrSrcTypeFromName(vLyrNamesToRemove[i].c_str());
    }

    int nRet;

    // remove lyrics3v2 lyrics
    if (isFlagSet(uAllLstFlag, LST_LYRICS3V2)) {
        CLyrics3v2 lyrics3v2;
        nRet = lyrics3v2.clearLyricsInfo(szMediaFile);
        if (nRet != ERR_OK && nRet != ERR_NOT_FIND_LRC3V2) {
            return nRet;
        }
    }

    // remove ID3v2 embedded lyrics
    if ((uAllLstFlag & LST_ID3V2) > 0) {
        CID3v2IF id3v2(ED_SYSDEF);
        nRet = id3v2.open(szMediaFile, true, false);
        if (nRet != ERR_OK) {
            return nRet;
        }

        bool bRemoved = false;
        for (uint32_t i = 0; i < vLyrNamesToRemove.size(); i++) {
            if ((lyrSrcTypeFromName(vLyrNamesToRemove[i].c_str()) & LST_ID3V2) > 0) {
                nRet = id3v2.removeLyrics(vLyrNamesToRemove[i].c_str());
                if (nRet == ERR_OK) {
                    bRemoved = true;
                }
            }
        }

        if (bRemoved) {
            nRet = id3v2.save();
            if (nRet == ERR_OK && succeededCount) {
                (*succeededCount)++;
            }
            return nRet;
        }

        return ERR_OK;
    }

    // remove M4a lyrics
    nRet = ERR_OK;
    if (isFlagSet(uAllLstFlag, LST_M4A_LYRICS)) {
        CM4aTag tag;
        nRet = tag.open(szMediaFile, true, false);
        if (nRet == ERR_OK) {
            nRet = tag.removeLyrics();
            if (nRet == ERR_OK) {
                nRet = tag.saveClose();
                if (nRet == ERR_OK && succeededCount) {
                    (*succeededCount)++;
                }
            }
        }
    }

    return nRet;
}
