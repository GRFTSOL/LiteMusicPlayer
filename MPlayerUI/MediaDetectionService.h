#pragma once

class CMediaDetectionService {
public:
    CMediaDetectionService(void);
    virtual ~CMediaDetectionService(void);

    void addMediaInDir(cstr_t szDir);
    void addMedia(vector<string> &vFiles);

};

extern CMediaDetectionService g_mediaDetectionService;
