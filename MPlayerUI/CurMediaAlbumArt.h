#pragma once

#include "../MediaTags/MediaTags.h"
#include "../ImageLib/RawImageData.h"


class CCurMediaAlbumArt {
public:
    CCurMediaAlbumArt();
    virtual ~CCurMediaAlbumArt();

    void reset();
    void restartLoop();

    RawImageDataPtr loadNext();

    int getPicCount();

public:
    int                         m_idxEmbeddedPicture = 0;
    int                         m_idxFilePicture = 0;
    // Album art in song file's directory.
    VecStrings                  m_vAlbumPicFiles;

};
