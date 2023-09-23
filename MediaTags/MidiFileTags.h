#pragma once

#include "IMediaFileTags.hpp"


namespace MediaTags {

class MidiFileTags : public IMediaFileTags {
public:
    // 返回支持的媒体文件扩展名，不包含 '.'
    VecStrings getSupportedExtNames() override { return { "mid", "kar" }; }

    string getFileKind(const StringView &fileExt) override;

    int getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) override;

    VecStrings enumLyrics(cstr_t fileName) override;
    int getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) override;

};

}
