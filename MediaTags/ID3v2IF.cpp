#include "ID3v2IF.h"
#include "ID3/ID3v1.h"
#include "ID3/ID3v2FrameParser.h"
#include "MediaTags.h"
#include "../ImageLib/RawImageData.h"


#define SZ_ID3V2_LYR_USER_DEFINED   "LYRICS"

bool isUnsynchLyricsFrameID(uint32_t nFrameID) {
    return nFrameID == ID3V2_2_USLT || nFrameID == ID3V2_3_USLT;
}

bool isSynchLyricsFrameID(uint32_t nFrameID) {
    return nFrameID == ID3V2_2_SYLT || nFrameID == ID3V2_3_SYLT;
}

void addEmbeddedLyrics(VecStrings &vNames, string &name) {
    int next = 0;
    for (int i = 0; i < vNames.size(); i++) {
        if (startsWith(vNames[i].c_str(), name.c_str())) {
            next++;
        }
    }

    if (next > 0) {
        name += stringPrintf("/%d", next).c_str();
    }
    vNames.push_back(name);
}

//////////////////////////////////////////////////////////////////////////
//
CID3v2IF::CID3v2IF(CharEncodingType encoding) : CID3v2(encoding) {
}

CID3v2IF::~CID3v2IF() {
}

int CID3v2IF::listLyrics(VecStrings &vLyrNames) {
    int nRet;

    for (FrameIterator it = frameBegin(); it != frameEnd(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (isUnsynchLyricsFrameID(pFrame->m_framehdr.nFrameID)) {
            CID3v2FrameParserUnsynLyrics parser(m_Encoding);
            ID3v2UnsynchLyrics unsyncLyrics;

            nRet = parser.parseInfoOnly(pFrame, unsyncLyrics);
            if (nRet == ERR_OK) {
                string name = string(SZ_SONG_ID3V2_USLT) + unsyncLyrics.m_language;
                addEmbeddedLyrics(vLyrNames, name);
            }
        } else if (isSynchLyricsFrameID(pFrame->m_framehdr.nFrameID)) {
            CID3v2FrameParserSynLyrics parser(m_Encoding);
            ID3v2SynchLyrics syncLyrics;

            nRet = parser.parseInfoOnly(pFrame, syncLyrics);
            if (nRet == ERR_OK && syncLyrics.m_byContentType == ID3v2SynchLyrics::CT_LYRICS) {
                string name = string(SZ_SONG_ID3V2_SYLT) + syncLyrics.m_language;
                addEmbeddedLyrics(vLyrNames, name);
            }
        } else if (pFrame->m_framehdr.nFrameID == ID3V2_2_USER_TEXT
            || pFrame->m_framehdr.nFrameID == ID3V2_3_USER_TEXT) {
            ID3v2TextUserDefined lyrics;
            CID3v2FrameParserTextUserDefined parser(m_Encoding);

            nRet = parser.parse(pFrame, lyrics);
            if (nRet == ERR_OK
                && strcasecmp(SZ_ID3V2_LYR_USER_DEFINED, lyrics.getDescription()) == 0) {
                vLyrNames.push_back(SZ_SONG_ID3V2_LYRICS);
            }
        }
    }

    return ERR_OK;
}

int CID3v2IF::getSyncLyrics(ID3v2SynchLyrics &lyrics, cstr_t szName) {
    FrameIterator it = getSyncLyricsFrame(szName);
    if (it == frameEnd()) {
        return ERR_NOT_FOUND;
    }

    CID3v2FrameParserSynLyrics parser(m_Encoding);

    int nRet = parser.parse(*it, lyrics);
    if (nRet != ERR_OK) {
        DBG_LOG1("Failed to parse Sync Lyrics frame: %s", Error2Str(nRet).c_str());
    }

    return nRet;
}

int CID3v2IF::getUnsyncLyrics(ID3v2UnsynchLyrics &lyrics, cstr_t szName) {
    FrameIterator it = getUnsyncLyricsFrame(szName);
    if (it == frameEnd()) {
        return ERR_NOT_FOUND;
    }

    CID3v2FrameParserUnsynLyrics parser(m_Encoding);

    int nRet = parser.parse(*it, lyrics);
    if (nRet != ERR_OK) {
        DBG_LOG1("Failed to parse Sync Lyrics frame: %s", Error2Str(nRet).c_str());
    }

    return nRet;
}

int CID3v2IF::getUserDefLyrics(ID3v2TextUserDefined &lyrics) {
    FrameIterator it = getUserDefLyricsFrame();
    if (it == frameEnd()) {
        return ERR_NOT_FOUND;
    }

    CID3v2FrameParserTextUserDefined parser(m_Encoding);
    return parser.parse(*it, lyrics);
}

int CID3v2IF::removeLyrics(cstr_t szName) {
    if (isEmptyString(szName)) {
        return ERR_NOT_FOUND;
    }

    FrameIterator it = m_listFrames.end();
    if (startsWith(szName, SZ_SONG_ID3V2_USLT)) {
        it = getUnsyncLyricsFrame(szName);
    } else if (startsWith(szName, SZ_SONG_ID3V2_SYLT)) {
        it = getSyncLyricsFrame(szName);
    } else if (startsWith(szName, SZ_SONG_ID3V2_LYRICS)) {
        it = getUserDefLyricsFrame();
    }

    if (it == m_listFrames.end()) {
        return ERR_NOT_FOUND;
    }

    frameRemove(it);

    return ERR_OK;
}

int CID3v2IF::setSynchLyrics(cstr_t szName, ID3v2SynchLyrics &lyrics) {
    uint32_t nFrameID = isID3v2_2() ? (int)ID3V2_2_SYLT : (int)ID3V2_3_SYLT;
    CID3v2Frame *pTargFrame = nullptr;
    FrameIterator it = getSyncLyricsFrame(szName);
    if (it == m_listFrames.end()) {
        // not found, so add it.
        pTargFrame = new CID3v2Frame(nFrameID);
        frameAdd(pTargFrame);
    } else {
        pTargFrame = *it;
    }

    string language;
    int index;
    if (getEmbeddedLyricsUrlInfo(szName, language, index) && language.size() > 0) {
        lyrics.m_language = language;
    }

    // do update
    pTargFrame->m_frameData.clear();
    CID3v2FrameParserSynLyrics parser(m_Encoding);
    return parser.toFrameData(lyrics, pTargFrame);
}

int CID3v2IF::setUnsynchLyrics(cstr_t szName, cstr_t szDesc, cstr_t szLyrics) {
    ID3v2UnsynchLyrics unsyncLyr;
    int index;

    getEmbeddedLyricsUrlInfo(szName, unsyncLyr.m_language, index);
    if (unsyncLyr.m_language.empty()) {
        unsyncLyr.m_language = "eng";
    }
    unsyncLyr.setValueAndOptimizeEncoding(szDesc, szLyrics);

    return setUnsynchLyrics(szName, unsyncLyr);
}

int CID3v2IF::setUnsynchLyrics(cstr_t szName, ID3v2UnsynchLyrics &lyrics) {
    uint32_t nFrameID = isID3v2_2() ? (int)ID3V2_2_USLT: (int)ID3V2_3_USLT;
    CID3v2Frame *pTargFrame = nullptr;
    FrameIterator it = getUnsyncLyricsFrame(szName);
    if (it == m_listFrames.end()) {
        // not found, so add it.
        pTargFrame = new CID3v2Frame(nFrameID);
        frameAdd(pTargFrame);
    } else {
        pTargFrame = *it;
    }

    // do update
    pTargFrame->m_frameData.clear();
    CID3v2FrameParserUnsynLyrics parser(m_Encoding);
    int nRet = parser.toFrameData(lyrics, pTargFrame);
    return nRet;
}

int CID3v2IF::setUserDefLyrics(cstr_t szLyrics) {
    uint32_t nFrameID = isID3v2_2() ? (int)ID3V2_2_USER_TEXT : (int)ID3V2_3_USER_TEXT;
    CID3v2Frame *pTargFrame = nullptr;
    FrameIterator it = getUserDefLyricsFrame();
    if (it == m_listFrames.end()) {
        // not found, so add it.
        pTargFrame = new CID3v2Frame(nFrameID);
        frameAdd(pTargFrame);
    } else {
        pTargFrame = *it;
    }

    ID3v2TextUserDefined text;
    text.setValueAndOptimizeEncoding(SZ_ID3V2_LYR_USER_DEFINED, szLyrics);

    // do update
    pTargFrame->m_frameData.clear();
    CID3v2FrameParserTextUserDefined parser(m_Encoding);
    int nRet = parser.toFrameData(text, pTargFrame);
    return nRet;
}

uint32_t _vTextFrameIDV22[] = {
    ID3V2_2_ARTIST, ID3V2_2_TITLE, ID3V2_2_ALBUM,
    ID3V2_2_TRACK, ID3V2_2_YEAR, ID3V2_2_GENRE
};

uint32_t _vTextFrameIDV23[] = {
    ID3V2_3_ARTIST, ID3V2_3_TITLE, ID3V2_3_ALBUM,
    ID3V2_3_TRACK, ID3V2_3_YEAR, ID3V2_3_GENRE
};

// TCOM    : Composer
// TPOS :
// TPE1 : Artist
// TPE2 : Album artist
// TALB : Album
// TIT2 : Title
// TRCK : Track
// TYER : Year
// COMM : Comment
// TCON : Genre
// APIC : pic
// USLT : unsyn lyrics
// TENC : encoded
// WXXX : url
// TCOP : copy right
// TOPE : Orig. artist
// TPUB : Publisher
// TBPM : bpm
int CID3v2IF::getTags(BasicMediaTags &tags) {
    string *vStr[] = {
        &tags.artist, &tags.title, &tags.album, &tags.trackNo, &tags.year, &tags.genre
    };

    uint32_t *vTextFrameID = isID3v2_2() ? _vTextFrameIDV22 : _vTextFrameIDV23;

        // read general text frames
    for (int i = 0; i < CountOf(_vTextFrameIDV22); i++) {
        auto frame = findFrame(vTextFrameID[i]);
        if (frame) {
            CID3v2FrameParserText txtParser(m_Encoding);
            ID3v2Text txt;
            auto ret = txtParser.parse(frame, txt);
            if (ret == ERR_OK) {
                *vStr[i] = txt.getValue();
            }
        }
    }

    // read comment
    uint32_t frameID = isID3v2_2() ? (uint32_t)ID3V2_2_COMMENT : (uint32_t)ID3V2_3_COMMENT;
    auto frame = findFrame(frameID);
    if (frame) {
        CID3v2FrameParserComment parser(m_Encoding);
        int ret = parser.parse(frame);
        if (ret == ERR_OK) {
            tags.comments = parser.m_strText.c_str();
        }
    }

    // read year
    if (tags.year.empty()) {
        uint32_t frameID = isID3v2_2() ? (uint32_t)ID3V2_2_YEAR2 : (uint32_t)ID3V2_3_YEAR2;
        auto frame = findFrame(frameID);
        if (frame) {
            CID3v2FrameParserText txtParser(m_Encoding);
            ID3v2Text txt;
            int ret = txtParser.parse(frame, txt);
            if (ret == ERR_OK) {
                tags.year = txt.getValue();
            }
        }
    }

    // read genre info
    if (!tags.genre.empty()) {
        auto &genre = tags.genre;
        if (isNumeric(genre.c_str())) {
            genre = CID3v1::getGenreDescription(atoi(genre.c_str()));
        } else if (genre[0] == '(') {
            cstr_t szGenre = genre.c_str();
            szGenre++;
            int nGenre = atoi(szGenre);
            while (isdigit(*szGenre)) {
                szGenre++;
            }
            if (*szGenre == ')') {
                szGenre++;
                cstr_t szNewGenre;
                szNewGenre = CID3v1::getGenreDescription(nGenre);
                if (szNewGenre && strcmp(szNewGenre, szGenre) == 0) {
                    genre = szNewGenre;
                }
            }
        }
    }

    return ERR_OK;
}

int CID3v2IF::setTags(const BasicMediaTags &tags) {
    uint32_t nFrameID;

    cstr_t    vTextFrameValues[CountOf(_vTextFrameIDV22)] = {
        tags.artist.c_str(), tags.title.c_str(), tags.album.c_str(),
        tags.trackNo.c_str(), tags.year.c_str(), tags.genre.c_str()
    };

    for (int i = 0; i < CountOf(vTextFrameValues); i++) {
        if (vTextFrameValues[i]) {
            if (isID3v2_2()) {
                nFrameID = _vTextFrameIDV22[i];
            } else {
                nFrameID = _vTextFrameIDV23[i];
            }
            if (isEmptyString(vTextFrameValues[i])) {
                removeFrame(nFrameID);
            } else {
                updateTextFrame(nFrameID, vTextFrameValues[i]);
            }
        }
    }

    // update comment
    if (isEmptyString(tags.comments.c_str())) {
        removeGeneralCommentFrame();
    } else {
        updateGeneralCommentFrame(tags.comments.c_str());
    }

    return ERR_OK;
}

int CID3v2IF::updateUserDefinedTextFrameByDesc(ID3v2TextUserDefined &text) {
    int nRet;
    CID3v2Frame *pFrame = nullptr;
    FrameIterator it;
    uint32_t nFrameID;
    CID3v2FrameParserTextUserDefined parser(m_Encoding);

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_USER_TEXT;
    } else {
        nFrameID = ID3V2_3_USER_TEXT;
    }

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;
        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            ID3v2TextUserDefined temp;
            nRet = parser.parse(pFrame, temp);
            if (nRet == ERR_OK) {
                if (strcmp(text.getDescription(), temp.getDescription()) == 0) {
                    break;
                }
            }
        }
    }

    if (it == frameEnd()) {
        // not found, so add it.
        pFrame = new CID3v2Frame(nFrameID);
        frameAdd(pFrame);
    }

    // do update
    assert(pFrame);
    pFrame->m_frameData.clear();
    return parser.toFrameData(text, pFrame);
}

int CID3v2IF::updateGeneralCommentFrame(cstr_t szText) {
    CID3v2Frame *pFrame;
    FrameIterator it;
    uint32_t nFrameID;
    CID3v2FrameParserComment parser(m_Encoding);

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_COMMENT;
    } else {
        nFrameID = ID3V2_3_COMMENT;
    }

    pFrame = findFrame(nFrameID);
    if (!pFrame) {
        // not found, so add it.
        pFrame = new CID3v2Frame(nFrameID);
        frameAdd(pFrame);

        if (isAnsiStr(szText)) {
            parser.m_encodingType = IET_ANSI;
        } else {
            parser.m_encodingType = IET_UCS2;
        }
    }

    parser.setText(nullptr, szText);

    // do update
    pFrame->m_frameData.clear();
    return parser.toFrame(pFrame);
}

// what is a general comment frame?
// 1) language is empty.
// 2) description is empty.
int CID3v2IF::removeGeneralCommentFrame() {
    int nRet;
    CID3v2Frame *pFrame;
    FrameIterator it;
    uint32_t nFrameID;
    CID3v2FrameParserComment parser(m_Encoding);

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_COMMENT;
    } else {
        nFrameID = ID3V2_3_COMMENT;
    }

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;

        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            nRet = parser.parse(pFrame);
            if (nRet == ERR_OK) {
                if (parser.m_language.empty() && parser.m_strShortDesc.empty()) {
                    // find it, and remove it.
                    frameRemove(it);
                    return ERR_OK;
                }
            }
        }
    }

    return ERR_NOT_FOUND;
}

int CID3v2IF::updateGeneralUrlFrame(cstr_t szText) {
    int nRet;
    CID3v2Frame *pFrame = nullptr;
    CID3v2FrameParserUserDefinedUrl parser(m_Encoding);
    uint32_t nFrameID;

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_URL;
    } else {
        nFrameID = ID3V2_3_URL;
    }

    pFrame = findFrame(nFrameID);
    if (!pFrame) {
        // not found, so add it.
        pFrame = new CID3v2Frame(nFrameID);
        frameAdd(pFrame);
    }

    parser.m_strUrl = szText;
    parser.m_textDesc.m_encodingType = IET_ANSI;

    // do update
    pFrame->m_frameData.clear();
    nRet = parser.toFrame(pFrame);
    return nRet;
}

int CID3v2IF::removeGeneralUrlFrame() {
    if (isID3v2_2()) {
        return removeFrame(ID3V2_2_URL);
    } else {
        return removeFrame(ID3V2_3_URL);
    }
}

void CID3v2IF::getPictures(ID3v2Pictures &pictures) {
    CID3v2FrameParserPic parser(m_Encoding);
    uint32_t nFrameID = isID3v2_2() ? ID3V2_2_PICTURE : ID3V2_3_PICTURE;

    pictures.free();

    for (auto frame : m_listFrames) {
        if (frame->m_framehdr.nFrameID == nFrameID) {
            auto pic = new ID3v2Pictures::ITEM;
            int ret = parser.parse(this, frame, pic);
            if (ret == ERR_OK) {
                pictures.m_vItems.push_back(pic);
            } else {
                ERR_LOG1("parse ID3v2 picture frame failed: %s", (cstr_t)Error2Str(ret));
                delete pic;
            }
        }
    }
}

void CID3v2IF::setPictures(const VecStrings &picturesData) {
    CID3v2FrameParserPic parser(m_Encoding);
    ID3v2Pictures pictures;
    getPictures(pictures);

    auto &pics = pictures.m_vItems;
    int i = 0;
    for (; i < pics.size() && i < picturesData.size(); i++) {
        pics[i]->m_buffPic = picturesData[i];
        pics[i]->picExtToMime(guessPictureDataExt(picturesData[i]));
        parser.toFrame(this, pics[i]->m_frame, pics[i]);
    }

    static ID3v2Pictures::PicType picTypes[] = {
        ID3v2Pictures::PT_COVER_FRONT, ID3v2Pictures::PT_COVER_BACK, ID3v2Pictures::PT_ARTIST,
        ID3v2Pictures::PT_MEDIA, ID3v2Pictures::PT_STD_ICON, ID3v2Pictures::PT_LEAD_ARTIST,
        ID3v2Pictures::PT_CONDUCTOR, ID3v2Pictures::PT_BAND, ID3v2Pictures::PT_COMPOSER,
        ID3v2Pictures::PT_LYRICIST, ID3v2Pictures::PT_RECORD_LOC, ID3v2Pictures::PT_DURING_LOC,
        ID3v2Pictures::PT_DURING_PERF, ID3v2Pictures::PT_MOVIE_CAPTURE, ID3v2Pictures::PT_COLORED_FISH,
        ID3v2Pictures::PT_ILLUSTRATION, ID3v2Pictures::PT_BAND_LOGO, ID3v2Pictures::PT_PUBLISHER_LOGO,
        ID3v2Pictures::PT_LEAFLET_PAGE,
    };

    if (i < pics.size()) {
        // 删除
        for (; i < pics.size(); i++) {
            removeFrameByUID(pics[i]->frameUID);
        }
    } else {
        // 添加
        uint32_t nFrameID = isID3v2_2() ? ID3V2_2_PICTURE : ID3V2_3_PICTURE;

        for (; i < picturesData.size(); i++) {
            auto frame = new CID3v2Frame(nFrameID);
            frameAdd(frame);

            ID3v2Pictures::ITEM pic;
            pic.m_buffPic = picturesData[i];
            pic.picExtToMime(guessPictureDataExt(picturesData[i]));
            pic.m_picType = picTypes[i % CountOf(picTypes)];

            // do update
            frame->m_frameData.clear();
            parser.toFrame(this, frame, &pic);
        }
    }
}

int CID3v2IF::updateTextFrame(uint32_t nFrameID, cstr_t szText) {
    int nRet;
    CID3v2Frame *pFrame = nullptr;
    ID3v2Text text;
    CID3v2FrameParserText parser(m_Encoding);

    pFrame = findFrame(nFrameID);
    if (!pFrame) {
        // not found, so add it.
        pFrame = new CID3v2Frame(nFrameID);

#ifdef DEBUG
        if (m_id3v2Header.byMajorVer == ID3v2Header::ID3V2_V2) {
            assert((nFrameID & 0xFF) == 0);
        }
#endif
        frameAdd(pFrame);
    }

    text.setValueAndOptimizeEncoding(szText);

    // do update
    pFrame->m_frameData.clear();
    nRet = parser.toFrameData(text, pFrame);
    return nRet;
}

CID3v2IF::FrameIterator CID3v2IF::getSyncLyricsFrame(cstr_t szName) {
    string language;
    int index;
    if (!getEmbeddedLyricsUrlInfo(szName, language, index)) {
        return m_listFrames.end();
    }

    int nFrameID = isID3v2_2() ? (int)ID3V2_2_SYLT : (int)ID3V2_3_SYLT;

    if (language.empty()) {
        return findFrameIter(nFrameID);
    }

    FrameIterator itLastFound = m_listFrames.end();
    for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (pFrame->getFrameID() == nFrameID
            && CID3v2FrameParserSynLyrics::isFrameLanguageSame(pFrame, language.c_str())) {
            itLastFound = it;
            if (index == 0) {
                return it;
            } else {
                index--;
            }
        }
    }

    return itLastFound;
}

CID3v2IF::FrameIterator CID3v2IF::getUnsyncLyricsFrame(cstr_t szName) {
    string language;
    int index;
    if (!getEmbeddedLyricsUrlInfo(szName, language, index)) {
        return m_listFrames.end();
    }

    int nFrameID = isID3v2_2() ? (int)ID3V2_2_USLT : (int)ID3V2_3_USLT;

    if (language.empty()) {
        return findFrameIter(nFrameID);
    }

    FrameIterator itLastFound = m_listFrames.end();
    for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (pFrame->getFrameID() == nFrameID
            && CID3v2FrameParserUnsynLyrics::isFrameLanguageSame(pFrame, language.c_str())) {
            itLastFound = it;
            if (index == 0) {
                return it;
            } else {
                index--;
            }
        }
    }

    return itLastFound;
}

CID3v2IF::FrameIterator CID3v2IF::getUserDefLyricsFrame() {
    uint32_t nFrameID = isID3v2_2() ? (int)ID3V2_2_USER_TEXT : (int)ID3V2_3_USER_TEXT;

    CID3v2FrameParserTextUserDefined parser(m_Encoding);
    for (FrameIterator it = m_listFrames.begin(); it != m_listFrames.end(); ++it) {
        CID3v2Frame *pFrame = *it;
        if (pFrame->getFrameID() != nFrameID) {
            continue;
        }

        ID3v2TextUserDefined temp;
        int nRet = parser.parse(*it, temp);
        if (nRet == ERR_OK) {
            if (strcmp(SZ_ID3V2_LYR_USER_DEFINED, temp.getDescription()) == 0) {
                return it;
            }
        }
    }

    return m_listFrames.end();
}
