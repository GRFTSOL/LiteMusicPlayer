#pragma once


void showMediaInfoDialog(CSkinWnd *pParent, const MediaPtr &media);

class CDlgMediaInfoPage;

enum MediaTagType {
    MTT_ID3V1                   = 1,
    MTT_ID3V2                   = 1 << 1,
    MTT_APEV1                   = 1 << 2,
    MTT_APEV2                   = 1 << 3,
};


#include "PreferencePageBase.h"


class CDlgMediaInfo : public CMPSkinWnd {
public:
    CDlgMediaInfo(const MediaPtr &media);
    ~CDlgMediaInfo(void);

    CUIObject *createUIObject(cstr_t className, CSkinContainer *container) override;

    void onSkinLoaded() override;

    void onDestroy() override;
    bool onCustomCommand(int nId) override;

    void onOK() override;

protected:
    void clear();
    void reloadMediaInfo();

protected:
    friend class CDlgMediaInfoPage;
    friend class CDlgMediaInfoPageBasic;
    friend class CDlgMediaInfoPageDetail;
    friend class CDlgMediaInfoPageLyrics;
    friend class CDlgMediaInfoPagePictures;

    bool                        _isTagsModified = false;
    bool                        _isLyricsModified = false;
    bool                        _isPicturesModified = false;
    bool                        _canModifyTags = false;
    bool                        _canModifyLyrics = false;
    bool                        _canModifyPictures = false;

    // basic and detail info
    BasicMediaTags              _tags;
    ExtendedMediaInfo           _info;

    // lyrics
    string                      _lyrics;
    string                      _lyricsOrg;

    // Array of pictures' data
    VecStrings                  _pictures;

    MediaPtr                    _media;

    string                      _strMediaFile;
    bool                        _bReadOnly;

    vector<class CDlgMediaInfoPage *>   _vInfoPages;

};
