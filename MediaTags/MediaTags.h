#pragma once

#include "ID3/ID3v1.h"
#include "ID3v2IF.h"
#include "M4aTag.h"


class MediaTags {
public:
    MediaTags(void);
    virtual ~MediaTags(void);

public:
    // pMediaLength: in ms
    static int getTagFast(cstr_t szFile, BasicMediaTags &tags);

    static int getAllTags(cstr_t szFile, CListTags &listTags);

    static bool isMediaTypeSupported(cstr_t szFile);

    static int getEmbeddedLyrics(cstr_t szFile, VecStrings &vLyricsNames);

    static bool isEmbeddedLyricsSupported(cstr_t szFile);
    static bool isID3v2TagSupported(cstr_t szFile);
    static bool isM4aTagSupported(cstr_t szFile);
    static bool isKarTagSupported(cstr_t szFile);

    static int removeEmbeddedLyrics(cstr_t szMediaFile, VecStrings &vLyrNamesToRemove, int *succeededCount = nullptr);

};
