#include "IMPlayer.h"
#include "Media.h"

/*
LPCXSTR CMPlayer::MediaAttributeName(MediaAttribute mediaAttri)
{
    static LPCXSTR        _szMediaAttrName[] = 
    {
        "Artist",
        "Album",
        "Title",
        "Track number",
        "Year",
        "Genre",
        "Comment",
        "length",
        "File size",
        "Time added",
        "Last played",
        "Rating",
        "Auto rating",
        "Times played",
        "Times play skipped",
        "Lyrics file",
        "Format",
        "Is Vbr",
        "bitrate",
        "BPS",
        "Channels",
        "Sample rate",
        "Extra Info",
    };

    if (mediaAttri >= 0 && mediaAttri < CountOf(_szMediaAttrName))
        return _szMediaAttrName[mediaAttri];
    else
        return "Undefined Name";
}*/

CMedia::CMedia()
{
    OBJ_REFERENCE_INIT

    m_nID = MEDIA_ID_INVALID;
    m_nTrackNumb = -1;
    m_nYear = -1;
     m_nLength = MEDIA_LENGTH_INVALID;
//     m_nFileSize = 0;
    m_nRating = 0;
    m_nPlayed = 0;
    m_nPlaySkipped = 0;
    m_nTimeAdded.getCurrentTime();
    /*m_nBitRate = 0;
    m_bIsVbr = false;
    m_nChannels = 2;
    m_nBPS = 16;*/
    m_nBitRate = 0;
    m_nBPS = 16;
    m_nSampleRate = 0;
    m_bInfoUpdated = false;
    m_bIsUserRating = false;
}

CMedia::~CMedia()
{
}

long CMedia::getID()
{
    return m_nID;
}

MLRESULT CMedia::getSourceUrl(IXStr *strUrl)
{
    strUrl->copy(m_strUrl.c_str());
    return ERR_OK;
}

MLRESULT CMedia::getArtist(IXStr *strArtist)
{
    strArtist->copy(m_strArtist.c_str());
    return ERR_OK;
}

MLRESULT CMedia::getTitle(IXStr *strTitle)
{
    strTitle->copy(m_strTitle.c_str());
    return ERR_OK;
}

MLRESULT CMedia::getAlbum(IXStr *strAlbum)
{
    strAlbum->copy(m_strAlbum.c_str());
    return ERR_OK;
}

long CMedia::getDuration()
{
    return m_nLength;
}

MLRESULT CMedia::setSourceUrl(LPCXSTR strUrl)
{
    m_strUrl = strUrl;
    return ERR_OK;
}

bool CMedia::isInfoUpdatedToMediaLib()
{
    return m_bInfoUpdated;
}

MLRESULT CMedia::setInfoUpdatedToMediaLib(bool bUpdated)
{
    m_bInfoUpdated = bUpdated;
    return ERR_OK;
}

void long2XStr(long value, IXStr *str)
{
    str->reserve(32);
    int size = snprintf(str->data(), str->capacity(), "%d", value);
    str->resize(size);
}

void mPTime2XStr(CMPTime &date, IXStr *str)
{
    MPTM        tm;

    if (date.getMPTM(&tm))
    {
        str->reserve(200);
        int size = snprintf(str->data(), str->capacity(), "%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
        str->resize(size);
    }
    else
        str->copy("-");
}

MLRESULT CMedia::getAttribute(MediaAttribute mediaAttr, IXStr *strValue)
{
    strValue->clear();

    switch (mediaAttr)
    {
    case MA_ARTIST        : strValue->copy(m_strArtist.c_str()); break;
    case MA_ALBUM        : strValue->copy(m_strAlbum.c_str()); break;
    case MA_TITLE        : strValue->copy(m_strTitle.c_str()); break;
    case MA_TRACK_NUMB    : if (m_nTrackNumb != -1) long2XStr(m_nTrackNumb, strValue); break;
    case MA_YEAR        : if (m_nYear != -1) long2XStr(m_nYear, strValue); break;
    case MA_GENRE        : strValue->copy(m_strGenre.c_str()); break;
    case MA_COMMENT        : strValue->copy(m_strComment.c_str()); break;
    case MA_BITRATE        : long2XStr(m_nBitRate, strValue); break;
    case MA_DURATION    : long2XStr(m_nLength, strValue); break;
    case MA_FILESIZE    : long2XStr(m_nFileSize, strValue); break;
    case MA_TIME_ADDED    : mPTime2XStr(m_nTimeAdded, strValue); break;
    case MA_TIME_PLAYED    : mPTime2XStr(m_nTimePlayed, strValue); break;
    case MA_RATING        : long2XStr(m_nRating, strValue); break;
    case MA_IS_USER_RATING    : long2XStr(m_bIsUserRating, strValue); break;
    case MA_TIMES_PLAYED: long2XStr(m_nPlayed, strValue); break;
    case MA_TIMES_PLAY_SKIPPED: long2XStr(m_nPlaySkipped, strValue); break;
    case MA_LYRICS_FILE    : strValue->copy(m_strLyricsFile.c_str()); break;

    // the following info will not stored in media library
    case MA_FORMAT        : strValue->copy(m_strFormat.c_str()); break;
    case MA_ISVBR        : long2XStr(m_bIsVbr, strValue); break;
    case MA_BPS            : long2XStr(m_nBPS, strValue); break;
    case MA_CHANNELS    : long2XStr(m_nChannels, strValue); break;
    case MA_SAMPLE_RATE    : long2XStr(m_nSampleRate, strValue); break;
    case MA_EXTRA_INFO    : strValue->copy(m_strExtraInfo.c_str()); break;
    default:
        return ERR_NOT_SUPPORT;
    }
    return ERR_OK;
}

MLRESULT CMedia::setAttribute(MediaAttribute mediaAttr, LPCXSTR szValue)
{
    switch (mediaAttr)
    {
    case MA_ARTIST        : m_strArtist = szValue; break;
    case MA_ALBUM        : m_strAlbum = szValue; break;
    case MA_TITLE        : m_strTitle = szValue; break;
    case MA_TRACK_NUMB    : m_nTrackNumb = atoi(szValue); break;
    case MA_YEAR        : m_nYear = atoi(szValue); break;
    case MA_GENRE        : m_strGenre = szValue; break;
    case MA_COMMENT        : m_strComment = szValue; break;
    case MA_BITRATE        : m_nBitRate = atoi(szValue); break;
    case MA_DURATION    : m_nLength = atoi(szValue); break;
    case MA_FILESIZE    : m_nFileSize = atoi(szValue); break;
//     case MA_TIME_ADDED    : m_nTimeAdded.dwTime = atoi(szValue); break;
//     case MA_TIME_PLAYED    : m_nTimePlayed.dwTime = atoi(szValue); break;
    case MA_RATING        : m_nRating = atoi(szValue); break;
    case MA_IS_USER_RATING    : m_bIsUserRating = tobool(atoi(szValue)); break;
    case MA_TIMES_PLAYED: m_nPlayed = atoi(szValue); break;
    case MA_TIMES_PLAY_SKIPPED: m_nPlaySkipped = atoi(szValue); break;
    case MA_LYRICS_FILE    : m_strLyricsFile = szValue; break;

    // the following info will not stored in media library
    case MA_FORMAT        : m_strFormat = szValue; break;
    case MA_ISVBR        : m_bIsVbr = tobool(atoi(szValue)); break;
    case MA_BPS            : m_nBPS = atoi(szValue); break;
    case MA_CHANNELS    : m_nChannels = atoi(szValue); break;
    case MA_SAMPLE_RATE    : m_nSampleRate = atoi(szValue); break;
    case MA_EXTRA_INFO    : m_strExtraInfo = szValue; break;
    default:
        return ERR_NOT_SUPPORT;
    }
    return ERR_OK;
}

MLRESULT CMedia::getAttribute(MediaAttribute mediaAttr, int *pnValue)
{
    switch (mediaAttr)
    {
    case MA_TRACK_NUMB          : *pnValue = m_nTrackNumb; break;
    case MA_YEAR                : *pnValue = m_nYear; break;
    case MA_BITRATE             : *pnValue = m_nBitRate; break;
    case MA_DURATION            : *pnValue = m_nLength; break;
    case MA_FILESIZE            : *pnValue = m_nFileSize; break;
    case MA_TIME_ADDED          : *pnValue = m_nTimeAdded.m_time; break;
    case MA_TIME_PLAYED         : *pnValue = m_nTimePlayed.m_time; break;
    case MA_RATING              : *pnValue = m_nRating; break;
    case MA_IS_USER_RATING      : *pnValue = m_bIsUserRating; break;
    case MA_TIMES_PLAYED        : *pnValue = m_nPlayed; break;
    case MA_TIMES_PLAY_SKIPPED  : *pnValue = m_nPlaySkipped; break;

    // the following info will not stored in media library
    case MA_ISVBR               : *pnValue = m_bIsVbr; break;
    case MA_BPS                 : *pnValue = m_nBPS; break;
    case MA_CHANNELS            : *pnValue = m_nChannels; break;
    case MA_SAMPLE_RATE         : *pnValue = m_nSampleRate; break;
    default                     : return ERR_NOT_SUPPORT;
    }
    return ERR_OK;
}

MLRESULT CMedia::setAttribute(MediaAttribute mediaAttr, int value)
{
    switch (mediaAttr)
    {
    case MA_TRACK_NUMB    : m_nTrackNumb = value; break;
    case MA_YEAR        : m_nYear = value; break;
    case MA_BITRATE        : m_nBitRate = value; break;
    case MA_DURATION    : m_nLength = value; break;
    case MA_FILESIZE    : m_nFileSize = value; break;
    case MA_TIME_ADDED    : m_nTimeAdded.m_time = value; break;
    case MA_TIME_PLAYED    : m_nTimePlayed.m_time = value; break;
    case MA_RATING        : m_nRating = value; break;
    case MA_IS_USER_RATING: m_bIsUserRating = tobool(value); break;
    case MA_TIMES_PLAYED: m_nPlayed = value; break;
    case MA_TIMES_PLAY_SKIPPED: m_nPlaySkipped = value; break;

    // the following info will not stored in media library
    case MA_ISVBR        : m_bIsVbr = tobool(value); break;
    case MA_BPS            : m_nBPS = value; break;
    case MA_CHANNELS    : m_nChannels = value; break;
    case MA_SAMPLE_RATE    : m_nSampleRate = value; break;
    default:
        return ERR_NOT_SUPPORT;
    }
    return ERR_OK;
}
