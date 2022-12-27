#pragma once

#include "MPTime.h"


class CMedia : public IMedia {
    OBJ_REFERENCE_DECL
public:
    CMedia();
    virtual ~CMedia();

    // the ID in media library, 0 for not exist in media library
    virtual int getID();

    virtual MLRESULT getSourceUrl(IString *strUrl);
    virtual MLRESULT getArtist(IString *strArtist);
    virtual MLRESULT getTitle(IString *strTitle);
    virtual MLRESULT getAlbum(IString *strAlbum);

    virtual int getDuration();

    virtual MLRESULT setSourceUrl(cstr_t strUrl);

    virtual bool isInfoUpdatedToMediaLib();
    virtual MLRESULT setInfoUpdatedToMediaLib(bool bUpdated);

    //
    // attribute methods
    //
    virtual MLRESULT getAttribute(MediaAttribute mediaAttr, IString *strValue);
    virtual MLRESULT setAttribute(MediaAttribute mediaAttr, cstr_t szValue);

    virtual MLRESULT getAttribute(MediaAttribute mediaAttr, int *pnValue);
    virtual MLRESULT setAttribute(MediaAttribute mediaAttr, int value);

protected:
    friend class CMediaLibrary;

    std::mutex                  m_mutexDataAccess;

    int                         m_nID;

    string                      m_strUrl;
    string                      m_strArtist;
    string                      m_strAlbum;
    string                      m_strTitle;
    int16_t                     m_nTrackNumb;
    int16_t                     m_nYear;
    string                      m_strGenre;
    string                      m_strComment;
    int                         m_nLength;
    uint32_t                    m_nFileSize;
    CMPTime                     m_nTimeAdded;
    CMPTime                     m_nTimePlayed;

    uint16_t                    m_nRating;          // 0 ~ 500
    bool                        m_bIsUserRating;
    bool                        m_bIsFileDeleted;
    int                         m_nPlayed;
    int                         m_nPlaySkipped;

    string                      m_strLyricsFile;

    string                      m_strFormat;
    int                         m_nBitRate;
    bool                        m_bIsVbr;
    uint8_t                     m_nChannels;
    uint8_t                     m_nBPS;
    bool                        m_bInfoUpdated;     // if InfoUpdated, it will be update to database when playing finished.
    uint32_t                    m_nSampleRate;
    string                      m_strExtraInfo;

};
