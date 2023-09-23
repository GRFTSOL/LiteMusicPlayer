//
//  MP3FileTags.hpp
//  Taglib
//
//  Created by henry_xiao on 2023/9/11.
//

#ifndef MP3FileTags_hpp
#define MP3FileTags_hpp

#include "IMediaFileTags.hpp"


namespace MediaTags {

class MP3FileTags : public IMediaFileTags {
public:
    // 返回支持的媒体文件扩展名，不包含 '.'
    VecStrings getSupportedExtNames() override;

    bool canSaveBasicTags() override { return true; }
    bool canSavePictures() override { return true; }
    VecStrings getSupportedSavingLyricsUrls() override;
    string getSuggestedSavingLyricsUrl() override;

    string getFileKind(const StringView &fileExt) override;

    int getTags(cstr_t fileName, BasicMediaTags &tagsOut, ExtendedMediaInfo &extendedInfoOut) override;
    int setBasicTags(cstr_t fileName, const BasicMediaTags &tags) override;

    VecStrings enumLyrics(cstr_t fileName) override;
    int getLyrics(cstr_t fileName, cstr_t lyricsUrl, bool isUseSpecifiedEncoding, CharEncodingType encodingSpecified, RawLyrics &rawLyricsOut) override;
    int setLyrics(cstr_t fileName, const RawLyrics &rawLyrics) override;
    int setLyrics(cstr_t fileName, const VecStrings &lyricsUrls, const RawLyrics &rawLyrics) override;
    int removeLyrics(cstr_t fileName, const VecStrings &lyrUrlsToRemove) override;

    int getPicture(cstr_t fileName, uint32_t index, string &imageDataOut) override;
    void getPictures(cstr_t fileName, VecStrings &vImagesDataOut) override;
    int setPictures(cstr_t fileName, const VecStrings &picturesData) override;

};

} // namespace MediaTags

#endif /* MP3FileTags_hpp */
