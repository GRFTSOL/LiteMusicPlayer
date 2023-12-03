#include "ID3v2FrameParser.h"
#include "ID3Helper.h"


// The ID3v2 tags needed to process
// 4.30.   Audio seek point index


//    frames that allow different types of text encoding contains a text
//    encoding description uint8_t. Possible encodings:
//
//      $00   ISO-8859-1 [ISO-8859-1]. Terminated with $00.
//      $01   UTF-16 [UTF-16] encoded Unicode [UNICODE] with BOM. All
//            strings in the same frame SHALL have the same byteorder.
//            Terminated with $00 00.
//      $02   UTF-16BE [UTF-16] encoded Unicode [UNICODE] without BOM.
//            Terminated with $00 00.
//      $03   UTF-8 [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.

//#define SizeOfUcs2StrWithBom(strUcs2)    ((strUcs2.size() + 1 + 1) * sizeof(WCHAR))

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserSynLyrics

int CID3v2FrameParserSynLyrics::parseInfoOnly(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics) {
    //     Text encoding        $xx
    //     Language             $xx xx xx
    //     Time stamp format    $xx
    //     Content type         $xx
    //     Content descriptor   <textstring> $00 (00)

    //   Content type:   $00 is other
    //                   $01 is lyrics
    //                   $02 is text transcription
    //                   $03 is movement/part name (e.g. "Adagio")
    //                   $04 is events (e.g. "Don Quijote enters the stage")
    //                   $05 is chord (e.g. "Bb F Fsus")
    //   Time stamp format is:
    //
    //     $01  Absolute time, 32 bit sized, using MPEG [MPEG] frames as unit
    //     $02  Absolute time, 32 bit sized, using milliseconds as unit

    //   The text that follows the frame header differs from that of the
    //   unsynchronised lyrics/text transcription in one major way. Each
    //   syllable (or whatever size of text is considered to be convenient by
    //   the encoder) is a null terminated string followed by a time stamp
    //   denoting where in the sound file it belongs. Each sync thus has the
    //   following structure:
    //
    //     Terminated text to be synced (typically a syllable)
    //     Sync identifier (terminator to above string)   $00 (00)
    //     Time stamp                                     $xx (xx ...)
    const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 6) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    synchLyrics.frameUID = frameHdr.frameUID;

    synchLyrics.m_encodingType = (ID3v2EncType)data[0];

    synchLyrics.m_language.assign(data + 1, 3);

    if (data[4] == 02) {
        synchLyrics.m_bTimeStampMs = true;
    } else {
        synchLyrics.m_bTimeStampMs = false;
    }

    synchLyrics.m_byContentType = data[5];

    data += 6;
    len -= 6;

    copyStrByEncodingAndBom(synchLyrics.m_strContentDesc, synchLyrics.m_encodingType, data, len, m_encoding);

    return ERR_OK;
}

int CID3v2FrameParserSynLyrics::parse(CID3v2Frame *pFrame, ID3v2SynchLyrics &synchLyrics) {
    //     Text encoding        $xx
    //     Language             $xx xx xx
    //     Time stamp format    $xx
    //     Content type         $xx
    //     Content descriptor   <textstring> $00 (00)

    //   Content type:   $00 is other
    //                   $01 is lyrics
    //                   $02 is text transcription
    //                   $03 is movement/part name (e.g. "Adagio")
    //                   $04 is events (e.g. "Don Quijote enters the stage")
    //                   $05 is chord (e.g. "Bb F Fsus")
    //   Time stamp format is:
    //
    //     $01  Absolute time, 32 bit sized, using MPEG [MPEG] frames as unit
    //     $02  Absolute time, 32 bit sized, using milliseconds as unit

    //   The text that follows the frame header differs from that of the
    //   unsynchronised lyrics/text transcription in one major way. Each
    //   syllable (or whatever size of text is considered to be convenient by
    //   the encoder) is a null terminated string followed by a time stamp
    //   denoting where in the sound file it belongs. Each sync thus has the
    //   following structure:
    //
    //     Terminated text to be synced (typically a syllable)
    //     Sync identifier (terminator to above string)   $00 (00)
    //     Time stamp                                     $xx (xx ...)
    const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 6) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    synchLyrics.frameUID = frameHdr.frameUID;

    synchLyrics.m_encodingType = (ID3v2EncType)data[0];

    synchLyrics.m_language.assign(data + 1, 3);

    if (data[4] == 02) {
        synchLyrics.m_bTimeStampMs = true;
    } else {
        synchLyrics.m_bTimeStampMs = false;
    }

    synchLyrics.m_byContentType = data[5];

    data += 6;
    len -= 6;

    int n = copyStrByEncodingAndBom(synchLyrics.m_strContentDesc, synchLyrics.m_encodingType, data, len, m_encoding);

    len -= n;
    data += n;

    while (len > 0) {
        while (data[0] == 0xA) {
            data++;
            len--;
            synchLyrics.addNewLineSynable();
            synchLyrics.m_bAllSyllableIsNewLine = false;
        }

        if (len < 5) {
            if (len != 0) {
                ERR_LOG1("Can't recognize all lyrics data, len: %d", len);
            }
            return ERR_OK;
        }

        string text;
        n = copyStrByEncodingAndBom(text, synchLyrics.m_encodingType, data, len, m_encoding);
        data += n;
        len -= n;

        int begTime = byteDataToUInt((uint8_t *)data, 4);
        data += 4;
        len -= 4;

        synchLyrics.addSynable(begTime, text);
    }

    return ERR_OK;
}

int CID3v2FrameParserSynLyrics::toFrameData(ID3v2SynchLyrics &synchLyrics, CID3v2Frame *pFrame) {
    string &buff = pFrame->m_frameData;

    buff += (char)synchLyrics.m_encodingType;

    //     Language             $xx xx xx
    if (synchLyrics.m_language.size() != 3) {
        synchLyrics.m_language = "XXX";
    }
    buff += synchLyrics.m_language;

    //     Time stamp format    $xx
    //       $02  Absolute time, 32 bit sized, using milliseconds as unit
    buff += (char)2;

    //     Content type         $xx
    assert(synchLyrics.m_byContentType == CT_LYRICS);
    buff += synchLyrics.m_byContentType;

    //     Content descriptor   <textstring> $00 (00)
    appendStrByEncodingAndBom(buff, synchLyrics.m_strContentDesc.c_str(), synchLyrics.m_encodingType, m_encoding);

    uint8_t byTime[10];
    for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = synchLyrics.m_vSynLyrics.begin();
    it != synchLyrics.m_vSynLyrics.end(); it++)
        {
        LrcSyllable &syllable = *it;
        if (syllable.bNewLine) {
            buff += (char)0xA;
        } else {
            appendStrByEncodingAndBom(buff, syllable.strText.c_str(), synchLyrics.m_encodingType, m_encoding);

            byteDataFromUInt(syllable.nTime, byTime, 4);
            buff.append((const char *)byTime, 4);
        }
    }

    return ERR_OK;
}

bool CID3v2FrameParserSynLyrics::isFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage) {
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 6) {
        return false;
    }

    if (strncmp(szLanguage, data + 1, 3) == 0
        && CID3v2FrameParserSynLyrics::CT_LYRICS == data[5]) {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserUnsynLyrics

int CID3v2FrameParserUnsynLyrics::parseInfoOnly(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics) {
    /*****************************************************************
<Header for 'Unsynchronised lyrics/text transcription', ID: "USLT"> 
    Text encoding        $xx 
    Language            $xx xx xx 
    Content descriptor    <text string according to encoding> $00 (00) 
    Lyrics/text            <full text string according to encoding> 
*****************************************************************/
    const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 4) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    unsynchLyrics.frameUID = frameHdr.frameUID;

    unsynchLyrics.m_encodingType = (ID3v2EncType)data[0];

    unsynchLyrics.m_language.assign(data + 1, 3);

    data += 4;
    len -= 4;

    copyStrByEncodingAndBom(unsynchLyrics.m_strContentDesc, unsynchLyrics.m_encodingType, data, len, m_encoding);

    return ERR_OK;
}

int CID3v2FrameParserUnsynLyrics::parse(CID3v2Frame *pFrame, ID3v2UnsynchLyrics &unsynchLyrics) {
    /*****************************************************************
<Header for 'Unsynchronised lyrics/text transcription', ID: "USLT"> 
    Text encoding        $xx 
    Language            $xx xx xx 
    Content descriptor    <text string according to encoding> $00 (00) 
    Lyrics/text            <full text string according to encoding> 
*****************************************************************/
    const ID3v2FrameHdr &frameHdr = pFrame->m_framehdr;
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 4) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    unsynchLyrics.frameUID = frameHdr.frameUID;

    unsynchLyrics.m_encodingType = (ID3v2EncType)data[0];

    unsynchLyrics.m_language.assign(data + 1, 3);

    data += 4;
    len -= 4;

    int n = copyStrByEncodingAndBom(unsynchLyrics.m_strContentDesc, unsynchLyrics.m_encodingType, data, len, m_encoding);

    data += n;
    len -= n;

    if (len > 0) {
        copyStrByEncodingAndBom(unsynchLyrics.m_strLyrics, unsynchLyrics.m_encodingType, data, len, m_encoding);
    }

    return ERR_OK;
}

int CID3v2FrameParserUnsynLyrics::toFrameData(ID3v2UnsynchLyrics &unsynchLyrics, CID3v2Frame *pFrame) {
    string &buff = pFrame->m_frameData;

    buff += (char)unsynchLyrics.m_encodingType;

    if (unsynchLyrics.m_language.size() != 3) {
        buff += "XXX";
    } else {
        buff += unsynchLyrics.m_language;
    }

    appendStrByEncodingAndBom(buff, unsynchLyrics.m_strContentDesc.c_str(), unsynchLyrics.m_encodingType, m_encoding);

    appendStrByEncodingAndBom(buff, unsynchLyrics.m_strLyrics.c_str(), unsynchLyrics.m_encodingType, m_encoding);

    return ERR_OK;
}

bool CID3v2FrameParserUnsynLyrics::isFrameLanguageSame(CID3v2Frame *pFrame, const char *szLanguage) {
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len <= 4) {
        return false;
    }

    if (strncmp(szLanguage, data + 1, 3) == 0) {
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////

void ID3v2Text::setValue(cstr_t szValue) {
    m_str = szValue;
}

void ID3v2Text::setValueAndOptimizeEncoding(cstr_t szValue) {
    if (m_encodingType == IET_ANSI && !isAnsiStr(szValue)) {
        m_encodingType = IET_UCS2;
    }
    setValue(szValue);
}

cstr_t ID3v2Text::getValue() {
    return m_str.c_str();
}

//////////////////////////////////////////////////////////////////////////

void ID3v2TextUserDefined::setValue(cstr_t szDescription, cstr_t szValue) {
    m_strValue = szValue;
    m_strDesc = szDescription;
}

void ID3v2TextUserDefined::setValueAndOptimizeEncoding(cstr_t szDescription, cstr_t szValue) {
    if (m_encodingType == IET_ANSI && (!isAnsiStr(szDescription) || !isAnsiStr(szValue))) {
        m_encodingType = IET_UCS2;
    }
    setValue(szDescription, szValue);
}

cstr_t ID3v2TextUserDefined::getDescription() {
    return m_strDesc.c_str();
}

cstr_t ID3v2TextUserDefined::getValue() {
    return m_strValue.c_str();
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserText

int CID3v2FrameParserText::parse(const CID3v2Frame *pFrame, ID3v2Text &text) {
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len < 1) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    text.m_encodingType = (ID3v2EncType)data[0];
    data++;

    copyStrByEncodingAndBom(text.m_str, text.m_encodingType, data, len, m_encoding);

    return ERR_OK;
}

int CID3v2FrameParserText::toFrameData(ID3v2Text &text, CID3v2Frame *pFrame) {
    string &buff = pFrame->m_frameData;

    buff.clear();
    buff += (char)text.m_encodingType;

    appendStrByEncodingAndBom(buff, text.m_str.c_str(), text.m_encodingType, m_encoding);

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// CID3v2FrameParserTextUserDefined

int CID3v2FrameParserTextUserDefined::parse(const CID3v2Frame *pFrame, ID3v2TextUserDefined &text) {
    const char *data = pFrame->m_frameData.c_str();
    int len = (int)pFrame->m_frameData.size();

    if (len < 3) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    text.m_encodingType = (ID3v2EncType)data[0];

    data++;
    len--;

    int n = copyStrByEncodingAndBom(text.m_strDesc, text.m_encodingType, data, len, m_encoding);
    data += n;
    len -= n;

    if (len <= 0) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    copyStrByEncodingAndBom(text.m_strValue, text.m_encodingType, data, len, m_encoding);

    return ERR_OK;
}

int CID3v2FrameParserTextUserDefined::toFrameData(ID3v2TextUserDefined &text, CID3v2Frame *pFrame) {
    string &buff = pFrame->m_frameData;

    buff.clear();
    buff += (char)text.m_encodingType;

    appendStrByEncodingAndBom(buff, text.m_strDesc.c_str(), text.m_encodingType, m_encoding);
    appendStrByEncodingAndBom(buff, text.m_strValue.c_str(), text.m_encodingType, m_encoding);

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserComment::parse(const CID3v2Frame *pFrame) {
    if (pFrame->m_frameData.size() < 6) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    const char *data = pFrame->m_frameData.c_str();

    m_encodingType = (ID3v2EncType)data[0];

    m_language.assign(data + 1, 3);

    size_t nPos = 4;

    nPos += copyStrByEncodingAndBom(m_strShortDesc, m_encodingType, data + nPos, (int)(pFrame->m_frameData.size() - nPos), m_encoding);
    if (nPos >= pFrame->m_frameData.size()) {
        return ERR_INVALID_ID3V2_FRAME;
    }
    copyStrByEncodingAndBom(m_strText, m_encodingType, data + nPos, (int)(pFrame->m_frameData.size() - nPos), m_encoding);

    return ERR_OK;
}

int CID3v2FrameParserComment::toFrame(CID3v2Frame *pFrame) {
    pFrame->m_frameData.clear();

    pFrame->m_frameData += char(m_encodingType);
    if (m_language.size() == 3) {
        pFrame->m_frameData.append(m_language);
    } else {
        pFrame->m_frameData.append("XXX");
    }

    appendStrByEncodingAndBom(pFrame->m_frameData, m_strShortDesc.c_str(), m_encodingType, m_encoding);
    appendStrByEncodingAndBom(pFrame->m_frameData, m_strText.c_str(), m_encodingType, m_encoding);

    return ERR_OK;
}

void CID3v2FrameParserComment::setText(cstr_t szShortDesc, cstr_t szText) {
    m_strText = szText;
    if (szShortDesc) {
        m_strShortDesc = szShortDesc;
    }
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserPic::parse(CID3v2 *pid3v2, const CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture) {
    if (pFrame->m_frameData.size() < 4) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    const char *data = pFrame->m_frameData.c_str();
    size_t nPos = 0;

    picture->m_frame = const_cast<CID3v2Frame *>(pFrame);
    picture->frameUID = pFrame->m_framehdr.frameUID;

    //  Text encoding      $xx
    picture->m_text.m_encodingType = (ID3v2EncType)data[nPos];
    nPos++;

    if (pid3v2->isID3v2_2()) {
        //  Image format       $xx xx xx
        picture->m_strMimeType.assign(data + nPos, 3);
        nPos += 3;
    } else {
        //  MIME type          <text string> $00
        nPos += copyAnsiStr(picture->m_strMimeType, data + nPos, (int)(pFrame->m_frameData.size() - nPos));
        if (nPos >= pFrame->m_frameData.size()) {
            return ERR_INVALID_ID3V2_FRAME;
        }
    }

    //  Picture type       $xx
    picture->m_picType = (ID3v2Pictures::PicType)(data[nPos]);
    nPos++;

    //  Description        <text string according to encoding> $00 (00)
    nPos += copyStrByEncodingAndBom(picture->m_text.m_str, picture->m_text.m_encodingType, data + nPos, (int)(pFrame->m_frameData.size() - nPos), m_encoding);
    if (nPos > pFrame->m_frameData.size()) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    //  Picture data       <binary data>
    picture->m_buffPic.append(data + nPos, pFrame->m_frameData.size() - nPos);

    return ERR_OK;
}

int CID3v2FrameParserPic::toFrame(CID3v2 *pid3v2, CID3v2Frame *pFrame, ID3v2Pictures::ITEM *picture) {
    pFrame->m_frameData.clear();

    pFrame->m_frameData += char(picture->m_text.m_encodingType);

    if (pid3v2->isID3v2_2()) {
        //  Image format       $xx xx xx
        char szPicExt[4];
        picture->mimeToPicExt(szPicExt);
        pFrame->m_frameData.append(szPicExt, 3);
    } else {
        //  MIME type          <text string> $00
        AppendAnsiStr(pFrame->m_frameData, picture->m_strMimeType);
    }

    pFrame->m_frameData += char(picture->m_picType);

    appendStrByEncodingAndBom(pFrame->m_frameData, picture->m_text.m_str.c_str(), picture->m_text.m_encodingType, m_encoding);

    pFrame->m_frameData.append(picture->m_buffPic.c_str(), picture->m_buffPic.size());

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int CID3v2FrameParserUserDefinedUrl::parse(const CID3v2Frame *pFrame) {
    if (pFrame->m_frameData.size() < 3) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    const char *data = pFrame->m_frameData.c_str();
    size_t nPos = 0;

    m_textDesc.m_encodingType = (ID3v2EncType)data[0];

    nPos++;
    nPos += copyStrByEncodingAndBom(m_textDesc.m_str, m_textDesc.m_encodingType, data + nPos, (int)(pFrame->m_frameData.size() - nPos), m_encoding);
    if (nPos >= pFrame->m_frameData.size()) {
        return ERR_INVALID_ID3V2_FRAME;
    }

    copyAnsiStr(m_strUrl, data + nPos, (int)(pFrame->m_frameData.size() - nPos));

    return ERR_OK;
}

int CID3v2FrameParserUserDefinedUrl::toFrame(CID3v2Frame *pFrame) {
    pFrame->m_frameData.clear();

    pFrame->m_frameData += char(m_textDesc.m_encodingType);

    appendStrByEncodingAndBom(pFrame->m_frameData, m_textDesc.m_str.c_str(), m_textDesc.m_encodingType, m_encoding);

    AppendAnsiStr(pFrame->m_frameData, m_strUrl);

    return ERR_OK;
}
