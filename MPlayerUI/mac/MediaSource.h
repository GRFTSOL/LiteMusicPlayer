#pragma once

enum MediaSourceType
{
    MST_ITUNES,
    MST_FILES,
};

class IMediaSource : public IListViewDataSource
{
public:
    IMediaSource(MediaSourceType type) : m_type(type) { }
    virtual ~IMediaSource();
    
    virtual bool updateLibrary(string &err) { return true; }

    MediaSourceType getType() const { return m_type; }

    virtual string getLocation(int nIndex) = 0;
    virtual bool getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength) = 0;

    virtual bool getEmbeddedLyrics(int nIndex, string &lyrics) = 0;
    virtual int setEmbeddedLyrics(int nIndex, string &lyrics) = 0;

    virtual int removeEmbeddedLyrics(int nIndex) = 0;

    virtual bool getBestMatchLyrics(int nIndex, string &lyrSrcPath);

    virtual void removeItem(int nIndex);

    virtual void setResult(int nIndex, cstr_t szResult);


    //
    // Common Methods for IListViewDataSource
    //

    virtual int getRowCount() const;
    
    virtual bool isRowSelected(int row) const;
    virtual void setRowSelectionState(int nIndex, bool bSelected);
    
    virtual int getItemImageIndex(int row);
    virtual bool setItemImageIndex(int nItem, int nImageIndex, bool bRedraw = true);
    
    virtual cstr_t getCellText(int row, int col);
    virtual CRawImage *getCellImage(int row, int col);

protected:
    class Item
    {
    public:
        int imageIndex;
        bool isSelected;
        string        result, lyrics;
        string        title;
        
        Item() { imageIndex = 0; isSelected = false; }
        virtual ~Item() { }

    };

    typedef vector<Item *>    VecTracks;

    enum
    {
        COL_MEDIA_FILE,
        COL_RESULT,
        COL_LYRICS,
    };

protected:
    MediaSourceType        m_type;
    VecTracks            m_vTracks;

};

class CFileMediaSource : public IMediaSource
{
public:
    CFileMediaSource() : IMediaSource(MST_FILES) { }
    
    void addFolder(cstr_t szFolder, bool bIncludeSubFolder);
    void addFiles(const VecStrings &vFiles);
    void addFile(cstr_t szFile);
    
    virtual string getLocation(int nIndex);
    virtual bool getMediaInfo(int nIndex, string &artist, string &album, string &title, string &location, uint32_t &nMediaLength);
    
    virtual bool getEmbeddedLyrics(int nIndex, string &lyrics);
    virtual int setEmbeddedLyrics(int nIndex, string &lyrics);
    
    virtual int removeEmbeddedLyrics(int nIndex);

protected:
    bool isFileExist(cstr_t szFile);

    virtual cstr_t getCellText(int row, int col);

protected:
    class ItemF : public Item
    {
    public:
        ItemF(string &file) { this->file = file; }
        string        file;
    };

};

IMediaSource *newMediaSource(MediaSourceType type);
