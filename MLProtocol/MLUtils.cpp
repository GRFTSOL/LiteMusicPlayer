#include "MLUtils.h"


MLFileType GetLyricsFileType(cstr_t szFile) {
    cstr_t szExt;

    szExt = strrchr(szFile, '.');
    if (szExt == nullptr) {
        return FT_UNKNOWN;
    }

    if (strcasecmp(szExt, ".lrc") == 0) {
        return FT_LYRICS_LRC;
    } else if (strcasecmp(szExt, ".txt") == 0) {
        return FT_LYRICS_TXT;
    } else if (strcasecmp(szExt, ".snc") == 0) {
        return FT_LYRICS_SNC;
    } else if (strcasecmp(szExt, ".srt") == 0) {
        return FT_SUBTITLE_SRT;
    }

    return FT_UNKNOWN;
}
