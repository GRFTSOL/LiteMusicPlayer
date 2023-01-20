#include "ID3v2IF.h"
#include "ID3/ID3v1.h"
#include "ID3/ID3v2FrameParser.h"
#include "MediaTags.h"


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
                string name = string(SZ_SONG_ID3V2_USLT) + unsyncLyrics.m_szLanguage;
                addEmbeddedLyrics(vLyrNames, name);
            }
        } else if (isSynchLyricsFrameID(pFrame->m_framehdr.nFrameID)) {
            CID3v2FrameParserSynLyrics parser(m_Encoding);
            ID3v2SynchLyrics syncLyrics;

            nRet = parser.parseInfoOnly(pFrame, syncLyrics);
            if (nRet == ERR_OK && syncLyrics.m_byContentType == ID3v2SynchLyrics::CT_LYRICS) {
                string name = string(SZ_SONG_ID3V2_SYLT) + syncLyrics.m_szLanguage;
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

    // do update
    pTargFrame->m_frameData.clear();
    CID3v2FrameParserSynLyrics parser(m_Encoding);
    return parser.toFrameData(lyrics, pTargFrame);
}

int CID3v2IF::setUnsynchLyrics(cstr_t szName, cstr_t szDesc, cstr_t szLyrics) {
    ID3v2UnsynchLyrics unsyncLyr;
    string language;
    int index;

    getEmbeddedLyricsNameInfo(szName, language, index);

    if (language.empty()) {
        language = "eng";
    }
    strcpy_safe(unsyncLyr.m_szLanguage, CountOf(unsyncLyr.m_szLanguage), language.c_str());
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
            parser.m_EncodingType = IET_ANSI;
        } else {
            parser.m_EncodingType = IET_UCS2LE_BOM;
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
                if (isEmptyString(parser.m_szLanguage) &&
                    parser.m_strShortDesc.empty()) {
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
    parser.m_textDesc.m_EncodingType = IET_ANSI;

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

int CID3v2IF::getPictures(ID3v2Pictures &pictures) {
    int nRet;
    FrameIterator it;
    CID3v2Frame *pFrame = nullptr;
    CID3v2FrameParserPic parser(m_Encoding);
    uint32_t nFrameID;
    ID3v2Pictures::ITEM *pic;

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_PICTURE;
    } else {
        nFrameID = ID3V2_3_PICTURE;
    }

    pictures.free();

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;
        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            pic = new ID3v2Pictures::ITEM;
            nRet = parser.parse(this, pFrame, pic);
            if (nRet == ERR_OK) {
                pictures.m_vItems.push_back(pic);
            } else {
                ERR_LOG1("parse ID3v2 picture frame failed: %s", (cstr_t)Error2Str(nRet));
                delete pic;
            }
        }
    }

    return ERR_OK;
}

int CID3v2IF::updatePictures(ID3v2Pictures &pictures) {
    int nRet;

    for (ID3v2Pictures::V_ITEMS::iterator it = pictures.m_vItems.begin();
    it != pictures.m_vItems.end(); ++it)
        {
        ID3v2Pictures::ITEM *pic = *it;
        if (pic->action == IFA_DEL) {
            nRet = removeFrameByUID(pic->frameUID);
        } else if (pic->action == IFA_MODIFY || pic->action == IFA_ADD) {
            nRet = updatePicFrame(pic);
        } else {
            continue;
        }
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    return ERR_OK;
}

int CID3v2IF::updatePicFrame(ID3v2Pictures::ITEM *pic) {
    int nRet;
    CID3v2Frame *pFrame = nullptr;
    CID3v2FrameParserPic parser(m_Encoding);
    uint32_t nFrameID;

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_PICTURE;
    } else {
        nFrameID = ID3V2_3_PICTURE;
    }

    if (pic->frameUID != -1) {
        pFrame = findFrameByUID(pic->frameUID, nFrameID);
    }
    if (!pFrame) {
        // not found, so add it.
        pFrame = new CID3v2Frame(nFrameID);
        frameAdd(pFrame);
    }

    // do update
    pFrame->m_frameData.clear();
    nRet = parser.toFrame(this, pFrame, pic);
    return nRet;
}

int CID3v2IF::removePicFrame(ID3v2FrameUID_t nFrameUID, uint32_t picType) {
    CID3v2Frame *pFrame = nullptr;
    FrameIterator it;
    uint32_t nFrameID;

    if (isID3v2_2()) {
        nFrameID = ID3V2_2_PICTURE;
    } else {
        nFrameID = ID3V2_3_PICTURE;
    }

    for (it = frameBegin(); it != frameEnd(); ++it) {
        pFrame = *it;

        if (pFrame->m_framehdr.nFrameID == nFrameID) {
            frameRemove(it);
            return ERR_OK;
        }
    }

    return ERR_NOT_FOUND;
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
    if (!getEmbeddedLyricsNameInfo(szName, language, index)) {
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
    if (!getEmbeddedLyricsNameInfo(szName, language, index)) {
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
