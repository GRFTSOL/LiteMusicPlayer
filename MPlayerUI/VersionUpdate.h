
#pragma once


uint32_t parseVersionStr(cstr_t version);

class CVersionInfo
{
public:
    int fromXML(SXNode *pNodeVer);

public:
    string              verNew;
    string              strNewVerDate;            // 新版发布日期
    string              strFeature;               // 新版特性

};

class CVersionUpdate  
{
public:
    CVersionUpdate();
    virtual ~CVersionUpdate();

    void checkNewVersion(bool bAutoCheck = true);

    virtual void onDownloadOK(CDownloadTask *pTask);

    int getUpdateInfo(CDownloadTask *pTask, CVersionInfo &versionInfo);

};
