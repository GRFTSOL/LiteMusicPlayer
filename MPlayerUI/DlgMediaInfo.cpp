#include "MPlayerAppBase.h"
#include "DlgMediaInfo.h"
#include "SkinImageListCtrl.h"

uint32_t getSupportedMTTByExt(cstr_t szMedia)
{
    cstr_t    szExt = fileGetExt(szMedia);

    if (strcmp(szExt, ".mp3") == 0)
        return MTT_ID3V1 | MTT_ID3V2;

    return 0;
}

class CDlgMediaInfoPage : public CSkinContainer
{
public:
    CDlgMediaInfoPage(cstr_t szPageId, cstr_t szAssociateTabButtonId) : CSkinContainer()
    {
        m_strAssociateTabButtonId = szAssociateTabButtonId;
        m_nAssociateTabButtonId = -1;
        m_bModified = false;
    }

    virtual void setParent(CDlgMediaInfo *pParent) { m_pDlgMediaInfo = pParent; }
    virtual void onUpdateView(bool bRedraw = true) { }
    virtual void onSave() { }

    void onInitialUpdate()
    {
        CSkinContainer::onInitialUpdate();

        m_nAssociateTabButtonId = getIDByName(m_strAssociateTabButtonId.c_str());
        onUpdateView(false);
    }

    void getFeildNewValue(int nID, string &strValue)
    {
        string        str;

        str = getUIObjectText(nID);
        if (strcmp(str.c_str(), strValue.c_str()) != 0)
        {
            m_bModified = true;
            strValue = str;
        }
    }

    bool isModified() { return m_bModified; }

    int getAssociateTabButtonId() const { return m_nAssociateTabButtonId; }

protected:
    CDlgMediaInfo        *m_pDlgMediaInfo;
    bool                m_bModified;
    string                  m_strAssociateTabButtonId;
    int                    m_nAssociateTabButtonId;

};

class CDlgMediaInfoPageBasic : public CDlgMediaInfoPage  
{
public:
    CDlgMediaInfoPageBasic() : CDlgMediaInfoPage("CID_C_MI_BASIC", "CMD_MI_BASIC")
    {
    }

    void onCreate()
    {
        CDlgMediaInfoPage::onCreate();

        GET_ID_BY_NAME3(CID_E_ARTIST, CID_E_ALBUM, CID_E_TITLE);
        GET_ID_BY_NAME4(CID_E_TRACK, CID_E_YEAR, CID_CB_GENRE, CID_E_COMMENT);

        CSkinComboBox        *pcbGenre;
        pcbGenre = (CSkinComboBox *)getUIObjectById(CID_CB_GENRE, CSkinComboBox::className());
        if (pcbGenre)
        {
            cstr_t *szGenre = CID3v1::GetAllGenreDescription();
            int        n = CID3v1::GetGenreCount();
            for (int i = 0; i < n; i++)
            {
                pcbGenre->addString(szGenre[i]);
            }
        }
    }

    void onUpdateView(bool bRedraw = true)
    {
        m_bModified = false;

        setUIObjectText(CID_E_ARTIST, m_pDlgMediaInfo->m_artist.c_str(), bRedraw);
        setUIObjectText(CID_E_ALBUM, m_pDlgMediaInfo->m_album.c_str(), bRedraw);
        setUIObjectText(CID_E_TITLE, m_pDlgMediaInfo->m_title.c_str(), bRedraw);
        setUIObjectText(CID_E_TRACK, m_pDlgMediaInfo->m_track.c_str(), bRedraw);
        setUIObjectText(CID_E_YEAR, m_pDlgMediaInfo->m_year.c_str(), bRedraw);
        setUIObjectText(CID_CB_GENRE, m_pDlgMediaInfo->m_genre.c_str(), bRedraw);
        setUIObjectText(CID_E_COMMENT, m_pDlgMediaInfo->m_comment.c_str(), bRedraw);
    }

    virtual void onSave()
    {
        getFeildNewValue(CID_E_ARTIST, m_pDlgMediaInfo->m_artist);
        getFeildNewValue(CID_E_ALBUM, m_pDlgMediaInfo->m_album);
        getFeildNewValue(CID_E_TITLE, m_pDlgMediaInfo->m_title);
        getFeildNewValue(CID_E_TRACK, m_pDlgMediaInfo->m_track);
        getFeildNewValue(CID_E_YEAR, m_pDlgMediaInfo->m_year);
        getFeildNewValue(CID_CB_GENRE, m_pDlgMediaInfo->m_genre);
        getFeildNewValue(CID_E_COMMENT, m_pDlgMediaInfo->m_comment);
        m_pDlgMediaInfo->m_bBasicInfoModified = m_bModified;
    }

    int                CID_E_ARTIST, CID_E_ALBUM, CID_E_TITLE;
    int                CID_E_TRACK, CID_E_YEAR, CID_CB_GENRE, CID_E_COMMENT;

};

class CDlgMediaInfoPageDetail : public CDlgMediaInfoPage  
{
public:
    CDlgMediaInfoPageDetail() : CDlgMediaInfoPage("CID_C_MI_DETAIL", "CMD_MI_DETAIL")
    {
    }

    void onCreate()
    {
        CDlgMediaInfoPage::onCreate();

        GET_ID_BY_NAME3(CID_E_COMPOSER, CID_E_COPYRIGHT, CID_E_ENCODED);
        GET_ID_BY_NAME4(CID_E_URL, CID_E_ORIG_ARTIST, CID_E_PUBLISHER, CID_E_BPM);
    }

    void onUpdateView(bool bRedraw = true)
    {
        m_bModified = false;

        setUIObjectText(CID_E_COMPOSER, m_pDlgMediaInfo->m_composer.c_str(), bRedraw);
        setUIObjectText(CID_E_COPYRIGHT, m_pDlgMediaInfo->m_copyRight.c_str(), bRedraw);
        setUIObjectText(CID_E_ENCODED, m_pDlgMediaInfo->m_encoded.c_str(), bRedraw);
        setUIObjectText(CID_E_URL, m_pDlgMediaInfo->m_url.c_str(), bRedraw);
        setUIObjectText(CID_E_ORIG_ARTIST, m_pDlgMediaInfo->m_origArtist.c_str(), bRedraw);
        setUIObjectText(CID_E_PUBLISHER, m_pDlgMediaInfo->m_publisher.c_str(), bRedraw);
        setUIObjectText(CID_E_BPM, m_pDlgMediaInfo->m_bpm.c_str(), bRedraw);
    }

    virtual void onSave()
    {
        getFeildNewValue(CID_E_COMPOSER, m_pDlgMediaInfo->m_composer);
        getFeildNewValue(CID_E_COPYRIGHT, m_pDlgMediaInfo->m_copyRight);
        getFeildNewValue(CID_E_ENCODED, m_pDlgMediaInfo->m_encoded);
        getFeildNewValue(CID_E_URL, m_pDlgMediaInfo->m_url);
        getFeildNewValue(CID_E_ORIG_ARTIST, m_pDlgMediaInfo->m_origArtist);
        getFeildNewValue(CID_E_PUBLISHER, m_pDlgMediaInfo->m_publisher);
        getFeildNewValue(CID_E_BPM, m_pDlgMediaInfo->m_bpm);
        m_pDlgMediaInfo->m_bDetailInfoModified = m_bModified;
    }

protected:
    int            CID_E_COMPOSER, CID_E_COPYRIGHT, CID_E_ENCODED;
    int            CID_E_URL, CID_E_ORIG_ARTIST, CID_E_PUBLISHER, CID_E_BPM;

};

class CDlgMediaInfoPageLyrics : public CDlgMediaInfoPage  
{
public:
    CDlgMediaInfoPageLyrics() : CDlgMediaInfoPage("CID_C_MI_LYRICS", "CMD_MI_LYRICS")
    {
    }

    void onCreate()
    {
        CDlgMediaInfoPage::onCreate();

        GET_ID_BY_NAME(CID_E_LYRICS);
    }

    void onUpdateView(bool bRedraw = true)
    {
        m_bModified = false;

        if (m_pDlgMediaInfo->m_strLyrics.size() > 0)
            setUIObjectText(CID_E_LYRICS, m_pDlgMediaInfo->m_strLyrics.c_str());
    }

    virtual void onSave()
    {
        string        strLyrics;
        strLyrics = getUIObjectText(CID_E_LYRICS);

        if (strcmp(strLyrics.c_str(), m_pDlgMediaInfo->m_strLyrics.c_str()) != 0)
        {
            m_pDlgMediaInfo->m_strLyrics = strLyrics.c_str();
            m_bModified = true;
            m_pDlgMediaInfo->m_bUnsyncLyricsModified = m_bModified;
        }
    }

protected:
    int                    CID_E_LYRICS;

};

//////////////////////////////////////////////////////////////////////////

class CDlgMediaInfoPagePictures : public CDlgMediaInfoPage  
{
public:
    CDlgMediaInfoPagePictures() : CDlgMediaInfoPage("CID_C_MI_PICTURES", "CMD_MI_PICTURES")
    {
    }

    void onCreate()
    {
        CDlgMediaInfoPage::onCreate();

        GET_ID_BY_NAME3(CID_ADD_PIC, CID_DEL_PIC, CID_SAVE_PIC_AS);

        m_pPicList = (CSkinImageListCtrl*)getUIObjectById("CID_LIST_PICS", CSkinImageListCtrl::className());
    }

    bool onCustomCommand(int nId)
    {
        if (nId == CID_ADD_PIC)
        {
            // add picture
            CFileOpenDlg    dlg(_TLT("Browse picture file"),
                "", "All picture files (*.bmp; *.jpg; *.gif; *.png)\0*.bmp;*.jpg;*.gif;*.png\0\0", 1);

            if (dlg.doModal(m_pSkin) != IDOK)
                return true;

            ID3v2Pictures::ITEM    *pic;

            pic = m_pDlgMediaInfo->m_pictures.appendNewPic();
            pic->action = IFA_ADD;
            pic->m_picType = ID3v2Pictures::PT_COVER_FRONT;
            pic->setImageFile(dlg.getOpenFile());
        }
        else if (nId == CID_DEL_PIC)
        {
            // delete picture
        }
        else if (nId == CID_SAVE_PIC_AS)
        {
            int nPos = -1;

            while (1)
            {
                nPos = m_pPicList->getNextSelectedItem(nPos);
                if (nPos == -1)
                    break;

                assert(nPos < m_pDlgMediaInfo->m_pictures.m_vItems.size());
                assert(m_pPicList->getItemCount() == m_pDlgMediaInfo->m_pictures.m_vItems.size());

                ID3v2Pictures::ITEM *pic = m_pDlgMediaInfo->m_pictures.m_vItems[nPos];
                char    szExtA[4];
                string    strExt;

                pic->mimeToPicExt(szExtA);
                CFileSaveDlg    dlg(_TLT("save picture as..."),
                    "", "All picture files (*.bmp; *.jpg; *.gif; *.png)\0*.bmp;*.jpg;*.gif;*.png\0\0", 1);

                if (dlg.doModal(m_pSkin) != IDOK)
                    break;
                strExt = ".";
                strExt += szExtA;

                string file = dlg.getSaveFile();
                fileSetExt(file, strExt.c_str());

                saveDataAsFile(file.c_str(), pic->m_buffPic.data(), pic->m_buffPic.size());
            }
        }
        else
            return CSkinContainer::onCustomCommand(nId);

        return true;
    }

    void onUpdateView(bool bRedraw = true)
    {
        m_bModified = false;

    }

    virtual void onSave()
    {
        if (m_pDlgMediaInfo->m_pictures.isModified())
            m_bModified = true;
        m_pDlgMediaInfo->m_bPicturesModified = m_bModified;
    }

protected:
    int                    CID_ADD_PIC, CID_DEL_PIC, CID_SAVE_PIC_AS;
    CSkinImageListCtrl    *m_pPicList;

};


//////////////////////////////////////////////////////////////////////////

CDlgMediaInfo::CDlgMediaInfo(IMedia *pMedia) : CMPSkinWnd()
{
    m_pOldPage = nullptr;
    m_pMedia = pMedia;

    m_vInfoPages.push_back(new CDlgMediaInfoPageBasic());
    m_vInfoPages.push_back(new CDlgMediaInfoPageDetail());
    m_vInfoPages.push_back(new CDlgMediaInfoPageLyrics());
    m_vInfoPages.push_back(new CDlgMediaInfoPagePictures());
}

CDlgMediaInfo::~CDlgMediaInfo(void)
{
}

void CDlgMediaInfo::onSkinLoaded()
{
    CMPSkinWnd::onSkinLoaded();

    for (int i = 0; i < (int)m_vInfoPages.size(); i++) {
        m_vInfoPages[i]->setParent(this);
        m_vInfoPages[i]->onInitialUpdate();
    }

    reloadMediaInfo();

    CSkinToolbar*pToolbar = (CSkinToolbar *)getUIObjectById(m_pSkinFactory->getIDByName("CID_TB_MEDIAINFO"),
        CSkinToolbar::className());
    if (pToolbar)
    {
        pToolbar->setCheck(m_pSkinFactory->getIDByName("CMD_MI_BASIC"), true, false);
    }
}

void CDlgMediaInfo::onDestroy()
{
    if (m_pMedia)
        m_pMedia.release();

    CMPSkinWnd::onDestroy();
}

void converID3v1Tag(char szTag[], int nLen, cstr_t szValue, int nEncoding)
{
    strcpy_safe(szTag, nLen, szValue);
}

void CDlgMediaInfo::onOK()
{
    bool        bModified = false;
    int            nRet;

    for (int i = 0; i < (int)m_vInfoPages.size(); i++)
    {
        m_vInfoPages[i]->onSave();
        if (m_vInfoPages[i]->isModified())
            bModified = true;
    }

    if (!bModified)
        return;

    //
    // update media info.
    //

    // update id3v1 tag
    if (m_bBasicInfoModified && (m_nSupportedMediaTagType & MTT_ID3V1))
    {
        CID3v1        id3v1;
        ID3V1        id3v1Info;

        memset(&id3v1Info, 0, sizeof(id3v1Info));

        converID3v1Tag(id3v1Info.szTitle, CountOf(id3v1Info.szTitle), m_title.c_str(), m_nEncodingOfConvertAnsi);
        converID3v1Tag(id3v1Info.szArtist, CountOf(id3v1Info.szArtist), m_artist.c_str(), m_nEncodingOfConvertAnsi);
        converID3v1Tag(id3v1Info.szAlbum, CountOf(id3v1Info.szAlbum), m_album.c_str(), m_nEncodingOfConvertAnsi);
        converID3v1Tag(id3v1Info.szComment, CountOf(id3v1Info.szComment), m_comment.c_str(), m_nEncodingOfConvertAnsi);
        converID3v1Tag(id3v1Info.szYear, CountOf(id3v1Info.szYear), m_year.c_str(), m_nEncodingOfConvertAnsi);
        id3v1Info.byTrack = atoi(m_track.c_str());
        id3v1Info.byGenre = CID3v1::getGenreIndex(m_genre.c_str());
        nRet = id3v1.saveTag(m_strMediaFile.c_str(), &id3v1Info);
        if (nRet != ERR_OK)
        {
            messageOut(ERROR2STR_LOCAL(nRet));
            return;
        }
    }

    // update id3v2 tag
    if (m_bDetailInfoModified || 
        m_bUnsyncLyricsModified ||
        m_bPicturesModified ||
        (m_bBasicInfoModified && (m_nSupportedMediaTagType & MTT_ID3V2)))
    {
        CID3v2IF        id3v2(m_nEncodingOfConvertAnsi);

        nRet = id3v2.open(m_strMediaFile.c_str(), true, true);
        if (nRet == ERR_OK)
        {
            // ID3v2 general tags
            if (m_bBasicInfoModified || m_bDetailInfoModified)
            {
                nRet = id3v2.setTags(m_artist.c_str(), m_title.c_str(), m_album.c_str(), m_comment.c_str(),
                    m_track.c_str(), m_year.c_str(), m_genre.c_str());
                //nRet = id3v2.SetExtraTags(m_composer.c_str(), m_encoded.c_str(), m_url.c_str(), m_copyRight.c_str(),
                //    m_origArtist.c_str(), m_publisher.c_str(), m_bpm.c_str());
                if (nRet != ERR_OK)
                    goto ERR_MSG_OUT_RET;
            }

            // Id3v2 lyrics
            if (m_bUnsyncLyricsModified)
            {
                nRet = id3v2.setUnsynchLyrics(m_strLyrName.c_str(), "", m_strLyrics.c_str());
                if (nRet != ERR_OK)
                    goto ERR_MSG_OUT_RET;
            }

            // id3v2 pictures
            if (m_bPicturesModified)
            {
                nRet = id3v2.updatePictures(m_pictures);
                if (nRet != ERR_OK)
                    goto ERR_MSG_OUT_RET;
            }
        }
        nRet = id3v2.save();
        if (nRet != ERR_OK)
            goto ERR_MSG_OUT_RET;
    }

    CMPSkinWnd::onOK();
    return;

ERR_MSG_OUT_RET:
    messageOut(ERROR2STR_LOCAL(nRet));
}

void CDlgMediaInfo::clear()
{    
    m_artist.resize(0);
    m_title.resize(0);
    m_album.resize(0);
    m_comment.resize(0);
    m_track.resize(0);
    m_year.resize(0);
    m_genre.resize(0);
    m_composer.resize(0);
    m_encoded.resize(0);
    m_url.resize(0);
    m_copyRight.resize(0);
    m_origArtist.resize(0);
    m_publisher.resize(0);
    m_bpm.resize(0);

    m_bBasicInfoModified = false;
    m_bDetailInfoModified = false;
    m_bUnsyncLyricsModified = false;
    m_bPicturesModified = false;

    m_strLyrics.clear();
}

#define IfNotEmptyThenSet(strSrc, strTarg)    if (!strSrc.empty())    strTarg = strSrc

void CDlgMediaInfo::reloadMediaInfo()
{
    //
    // get Media file name and info
    //
    CXStr        strMedia;
    MLRESULT    nRet;

    clear();

    if (!m_pMedia)
        return;

    m_nSupportedMediaTagType = 0;
    nRet = m_pMedia->getSourceUrl(&strMedia);
    if (nRet != ERR_OK)
        return;

    m_strMediaFile = strMedia.c_str();
    m_nSupportedMediaTagType = getSupportedMTTByExt(m_strMediaFile.c_str());

    // load id3v1 tag
    if (m_nSupportedMediaTagType & MTT_ID3V1)
    {
        CID3v1        id3v1;
        ID3V1        id3v1Info;
        nRet = id3v1.getTag(m_strMediaFile.c_str(), &id3v1Info);
        if (nRet == ERR_OK)
        {
            mbcsToUtf8(id3v1Info.szArtist, -1, m_artist, m_nEncodingOfConvertAnsi);
            mbcsToUtf8(id3v1Info.szTitle, -1, m_title, m_nEncodingOfConvertAnsi);
            mbcsToUtf8(id3v1Info.szAlbum, -1, m_album, m_nEncodingOfConvertAnsi);
            mbcsToUtf8(id3v1Info.szComment, -1, m_comment, m_nEncodingOfConvertAnsi);
            mbcsToUtf8(id3v1Info.szYear, -1, m_year, m_nEncodingOfConvertAnsi);

            m_track = CStrPrintf("%d", id3v1Info.byTrack).c_str();
            cstr_t    szGenre;
            szGenre = CID3v1::getGenreDescription(id3v1Info.byGenre);
            if (szGenre)
                mbcsToUtf8(szGenre, -1, m_genre, m_nEncodingOfConvertAnsi);            
        }
    }

    // load id3v2 tag
    if (m_nSupportedMediaTagType & MTT_ID3V2)
    {
        CID3v2IF        id3v2(m_nEncodingOfConvertAnsi);

        nRet = id3v2.open(m_strMediaFile.c_str(), false, false);
        if (nRet == ERR_OK)
        {
            string  artist,  title,  album,  comment;
            string  track,  year,  genre;

            // ID3v2 general tags
            nRet = id3v2.getTags(artist, title, album, comment,
                track, year, genre);//, m_composer,
                //m_encoded, m_url, m_copyRight, m_origArtist, m_publisher, m_bpm);
            if (nRet == ERR_OK)
            {
                IfNotEmptyThenSet(artist, m_artist);
                IfNotEmptyThenSet(title, m_title);
                IfNotEmptyThenSet(album, m_album);
                IfNotEmptyThenSet(comment, m_comment);
                IfNotEmptyThenSet(track, m_track);
                IfNotEmptyThenSet(year, m_year);
                IfNotEmptyThenSet(genre, m_genre);
            }

            // Id3v2 lyrics
            ID3v2UnsynchLyrics lyrics;
            nRet = id3v2.getUnsyncLyrics(lyrics);
            if (nRet == ERR_OK)
                m_strLyrics = lyrics.m_strLyrics.c_str();

            // id3v2 pictures
            id3v2.getPictures(m_pictures);
        }
    }
}

void showMediaInfoDialog(CSkinWnd *pParent, IMedia *pMedia)
{
    SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, _SZ_SKINWND_CLASS_NAME, 
        "MediaInfo.xml", pParent);

    CDlgMediaInfo *pWndMediaInfo = new CDlgMediaInfo(pMedia);
    skinWndStartupInfo.pSkinWnd = pWndMediaInfo;

    CSkinApp::getInstance()->getSkinFactory()->activeOrCreateSkinWnd(skinWndStartupInfo);
}
