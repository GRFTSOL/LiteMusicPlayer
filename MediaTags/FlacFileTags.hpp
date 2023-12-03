﻿//
//  FlacFileTags.hpp
//  Taglib
//
//  Created by henry_xiao on 2023/8/28.
//

#ifndef FlacFileTags_hpp
#define FlacFileTags_hpp

#include "IMediaFileTags.hpp"


namespace MediaTags {

class FlacFileTags : public IMediaFileTags {
public:
    // 返回支持的媒体文件扩展名，不包含 '.'
    VecStrings getSupportedExtNames() override { return { "flac" }; }

    bool canSaveBasicTags() override { return false; }

    string getFileKind(const StringView &fileExt) override;

    int getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) override;
    int setBasicTags(cstr_t fileName, const BasicMediaTags &tags) override { return ERR_NOT_SUPPORT_FILE_FORMAT; }

    VecStrings enumLyrics(cstr_t fileName) override;
    int getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) override;

    int getPicture(cstr_t fileName, uint32_t index, string &imageDataOut) override;
    void getPictures(cstr_t fileName, VecStrings &vImagesDataOut) override;

};

}

#endif /* FlacFileTags_hpp */
