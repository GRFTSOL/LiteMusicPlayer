#pragma once


void showMediaInfoDialog(CSkinWnd *pParent, IMedia *pMedia);

class CDlgMediaInfoPage;

enum MediaTagType
{
    MTT_ID3V1        = 1,
    MTT_ID3V2        = 1 << 1,
    MTT_APEV1        = 1 << 2,
    MTT_APEV2        = 1 << 3,
};

uint32_t getSupportedMTTByExt(cstr_t szMedia);


#include "PreferencePageBase.h"

class CDlgMediaInfo : public CMPSkinWnd
{
public:
    CDlgMediaInfo(IMedia *pMedia);
    ~CDlgMediaInfo(void);

    void onSkinLoaded();

    void onDestroy();

    void onOK();

protected:
    void clear();
    void reloadMediaInfo();

protected:
    friend class CDlgMediaInfoPage;
    friend class CDlgMediaInfoPageBasic;
    friend class CDlgMediaInfoPageDetail;
    friend class CDlgMediaInfoPageLyrics;
    friend class CDlgMediaInfoPagePictures;

    // basic and detail info
    string                m_artist,  m_title,  m_album,  m_comment;
    string                m_track,  m_year,  m_genre,  m_composer;
    string                m_encoded,  m_url,  m_copyRight,  m_origArtist,  m_publisher,  m_bpm;
    bool                m_bBasicInfoModified;
    bool                m_bDetailInfoModified;

    // lyrics
    string                m_strLyrName;
    string                m_strLyrics;
    bool                m_bUnsyncLyricsModified;

    // pictures
    ID3v2Pictures        m_pictures;
    bool                m_bPicturesModified;


protected:
    CMPAutoPtr<IMedia>    m_pMedia;

    string                m_strMediaFile;
    bool                m_bReadOnly;
    uint32_t                m_nSupportedMediaTagType;
    CharEncodingType        m_nEncodingOfConvertAnsi;

    vector<class CDlgMediaInfoPage *>    m_vInfoPages;
    class CDlgMediaInfoPage                *m_pOldPage;

};
