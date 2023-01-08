#pragma once

#include "../MediaTags/MediaTags.h"
#include "../ImageLib/RawImageData.h"


class CCurMediaAlbumArt {
public:
    CCurMediaAlbumArt();
    virtual ~CCurMediaAlbumArt();

    bool isLoaded() const { return m_bLoaded; }
    int load();

    void close();

    RawImageDataPtr loadAlbumArtByIndex(int nIndex);

    int getPicCount();

public:
    // V_PICS                m_vPics;
    ID3v2Pictures               m_id3v2Pic;

    // album art of song file directory.
    VecStrings                  m_vAlbumPicFile;

    bool                        m_bLoaded;

};
