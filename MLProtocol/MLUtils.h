#ifndef _MLUTILS_INC_
#define _MLUTILS_INC_

#include "../Utils/Utils.h"


enum MLFileType
{
    FT_UNKNOWN            = 0,
    FT_SUBTITLE_SRT        = 1,
    FT_LYRICS_TXT        = 1 << 1,
    FT_LYRICS_LRC        = 1 << 2,
    FT_LYRICS_SNC        = 1 << 3,
    FT_LYRICS            = FT_LYRICS_TXT | FT_SUBTITLE_SRT | FT_LYRICS_LRC,
};

MLFileType GetLyricsFileType(cstr_t szFile);


#endif // _MLUTILS_INC_
