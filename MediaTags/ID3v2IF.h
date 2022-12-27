#pragma once

#include "ID3/ID3v2.h"


class CID3v2IF : public CID3v2 {
public:
    CID3v2IF(CharEncodingType encoding);
    ~CID3v2IF();

    int listLyrics(VecStrings &vLyrNames);

    int getSyncLyrics(ID3v2SynchLyrics &lyrics, cstr_t szName = nullptr);
    int getUnsyncLyrics(ID3v2UnsynchLyrics &lyrics, cstr_t szName = nullptr);
    int getUserDefLyrics(ID3v2TextUserDefined &lyrics);

    int removeLyrics(cstr_t szName);

    int setSynchLyrics(cstr_t szName, ID3v2SynchLyrics &lyrics);
    int setUnsynchLyrics(cstr_t szName, cstr_t szDesc, cstr_t szLyrics);
    int setUserDefLyrics(cstr_t szLyrics);

    virtual int getTags(string &artist, string &title, string &album, string &comment,
        string &track, string &year, string &genre);

    virtual int setTags(cstr_t szArtist, cstr_t szTitle, cstr_t szAlbum, cstr_t szComment,
        cstr_t szTrack, cstr_t szYear, cstr_t szGenre);

    int updateUserDefinedTextFrameByDesc(ID3v2TextUserDefined &text);
    int removeUserDefinedTextFrameByDesc(cstr_t szDesc);

    int updateGeneralCommentFrame(cstr_t szText);
    int removeGeneralCommentFrame();

    int updateGeneralUrlFrame(cstr_t szText);
    int removeGeneralUrlFrame();

    int getPictures(ID3v2Pictures &pictures);
    int updatePictures(ID3v2Pictures &pictures);

    int updatePicFrame(ID3v2Pictures::ITEM *pic);
    int removePicFrame(ID3v2FrameUID_t nFrameUID, uint32_t picType);

protected:
    int updateTextFrame(uint32_t nFrameID, cstr_t szText);

    FrameIterator getSyncLyricsFrame(cstr_t szName);
    FrameIterator getUnsyncLyricsFrame(cstr_t szName);
    FrameIterator getUserDefLyricsFrame();

    int setUnsynchLyrics(cstr_t szName, ID3v2UnsynchLyrics &lyrics);

};
