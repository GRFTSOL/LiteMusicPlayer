#pragma once

#include "MediaTagTypes.hpp"
#include "LyricsData.h"


namespace MediaTags {

string getFileKind(cstr_t fileName);

int getTags(cstr_t fileName, BasicMediaTags &tags, ExtendedMediaInfo &extendedInfo);
bool canSaveBasicTags(cstr_t lyricsName);
int setBasicTags(cstr_t fileName, BasicMediaTags &tags);

bool isEmbeddedLyricsUrl(cstr_t lyricsName);

VecStrings getEmbeddedLyrics(cstr_t fileName);
int openEmbeddedLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut);
int saveEmbeddedLyrics(cstr_t fileName, const VecStrings &vLyricsUrls, const string &lyrics);
int saveEmbeddedLyrics(cstr_t fileName, const VecStrings &vLyricsUrls, const RawLyrics &rawLyrics);
// 保存一种或者多种嵌入到歌曲中的歌词
int saveEmbeddedLyrics(cstr_t fileName, const string &lyrics);

bool canSaveEmbeddedLyrics(cstr_t fileName);
VecStrings getSupportedEmbeddedLyricsUrls(cstr_t fileName);
string getSuggestedEmbeddedLyricsUrl(cstr_t fileName);

int removeEmbeddedLyrics(cstr_t mediaFile, VecStrings &vLyrNamesToRemove, int *succeededCount = nullptr);

int getEmbeddedPicture(cstr_t fileName, int index, string &imageDataOut);
void getEmbeddedPictures(cstr_t fileName, VecStrings &imageDataOut);
bool canSavePictures(cstr_t fileName);
int setEmbeddedPictures(cstr_t fileName, const VecStrings &pictures);

} // namespace MediaTags
