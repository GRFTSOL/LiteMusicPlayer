#pragma once

#include "ID3/ID3v1.h"
#include "ID3v2IF.h"
#include "M4aTag.h"


namespace MediaTags {

// pMediaLength: in ms
int getTags(cstr_t szFile, BasicMediaTags &tags, ExtendedMediaInfo &extendedInfo);
int setBasicTags(cstr_t szFile, BasicMediaTags &tags);

bool isMediaTypeSupported(cstr_t szFile);

int getEmbeddedLyrics(cstr_t szFile, VecStrings &vLyricsNames);

bool isEmbeddedLyricsSupported(cstr_t szFile);
bool isID3v2TagSupported(cstr_t szFile);
bool isM4aTagSupported(cstr_t szFile);
bool isKarTagSupported(cstr_t szFile);

int removeEmbeddedLyrics(cstr_t szMediaFile, VecStrings &vLyrNamesToRemove, int *succeededCount = nullptr);

} // namespace MediaTags
