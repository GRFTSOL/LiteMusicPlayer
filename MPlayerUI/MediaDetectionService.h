#pragma once

/*/////////////////////////////////////////////////////////////////////////

Detection the changes of media library or new media, add/update media library.

*//////////////////////////////////////////////////////////////////////////

class CMediaDetectionService
{
public:
    CMediaDetectionService(void);
    virtual ~CMediaDetectionService(void);

    void addMediaInDir(cstr_t szDir);
    void addMedia(vector<string> &vFiles);

};

extern CMediaDetectionService        g_mediaDetectionService;