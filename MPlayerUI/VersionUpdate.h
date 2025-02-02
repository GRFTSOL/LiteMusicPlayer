#pragma once


bool isNewVersion(cstr_t version);

class CVersionInfo {
public:
    string                      verNew;
    string                      releaseDate;      // 新版发布日期

};

class CVersionUpdate {
public:
    CVersionUpdate();
    virtual ~CVersionUpdate();

    void checkNewVersion(bool bAutoCheck = true);

    virtual void onDownloadOK(CDownloadTask *pTask);

    int getUpdateInfo(CDownloadTask *pTask, CVersionInfo &versionInfo);

};
