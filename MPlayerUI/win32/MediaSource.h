#pragma once

enum MediaSourceType
{
    MST_ITUNES,
    MST_FILES,
};

class IMediaSource
{
public:
    IMediaSource(MediaSourceType type) : m_type(type) { }
    virtual ~IMediaSource() { }

    MediaSourceType getType() const { return m_type; }

    virtual int getItemCount() = 0;
    virtual bool getLocation(int nIndex, string &location) = 0;
    virtual bool getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength) = 0;

    virtual bool getEmbeddedLyrics(int nIndex, string &lyrics) = 0;
    virtual int setEmbeddedLyrics(int nIndex, string &lyrics) = 0;

    virtual int removeEmbeddedLyrics(int nIndex) = 0;

    virtual bool getBestMatchLyrics(int nIndex, string &lyrSrcPath);

    virtual void removeItem(int nIndex) = 0;

protected:
    MediaSourceType        m_type;

};


#include "../pluginiTunes/iTunesCOMInterface.h"

class CiTunesMediaSource : public IMediaSource
{
public:
    CiTunesMediaSource();
    ~CiTunesMediaSource();

    bool updateLibrary(string &err);

    virtual int getItemCount();
    virtual bool getLocation(int nIndex, string &location);
    virtual bool getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength);

    virtual bool getEmbeddedLyrics(int nIndex, string &lyrics);
    virtual int setEmbeddedLyrics(int nIndex, string &lyrics);

    virtual int removeEmbeddedLyrics(int nIndex);

protected:
    struct Item
    {
        string        location, artist, album, title;
        uint32_t        duration;
        IITFileOrCDTrack *trackFile;
    };
    typedef vector<Item>    VecTracks;

    virtual void removeItem(int nIndex);

protected:
    virtual bool updateMediaInfo(Item &item);

protected:
    CMPAutoPtr<IiTunes>        m_iTunesApp;
    CMPAutoPtr<IITLibraryPlaylist> mainLibrary;
    CMPAutoPtr<IITTrackCollection> tracks;

    VecTracks                m_vTracks;

#ifdef _DEBUG
    uint32_t                m_threadId;
#endif

};

class CFileMediaSource : public IMediaSource
{
public:
    CFileMediaSource() : IMediaSource(MST_FILES) { }

    void addFolder(cstr_t szFolder, bool bIncludeSubFolder);
    void addFiles(const VecStrings &vFiles);
    void addFile(cstr_t szFile);

    virtual void close();

    virtual int getItemCount();
    virtual bool getLocation(int nIndex, string &location);
    virtual bool getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength);

    virtual bool getEmbeddedLyrics(int nIndex, string &lyrics);
    virtual int setEmbeddedLyrics(int nIndex, string &lyrics);

    virtual int removeEmbeddedLyrics(int nIndex);

    virtual void removeItem(int nIndex);

protected:
    void expandMediaFiles(VecStrings &vFiles);
    bool isFileExist(cstr_t szFile);

protected:
    struct Item
    {
        string        file, artist, album, title;
        bool        bInfoAvailable;
        uint32_t        nMediaLength;

        Item(string &file) {
            this->bInfoAvailable = false;
            this->file = file;
            this->nMediaLength = 0;
        }
    };
    typedef vector<Item>        VecItems;

    VecItems                m_vFiles;

};
