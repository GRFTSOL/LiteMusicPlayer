// VersionUpdate.cpp: implementation of the CVersionUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "../version.h"


#ifdef _MPLAYER
#define SOFTWARE_INFO        "zikiplayerinfo"
#define VER_FILE            "zpver.xml"
#else
#define SOFTWARE_INFO        "minilyricsinfo"
#define VER_FILE            "mlver.xml"
#endif


uint32_t parseVersionStr(cstr_t version) {
    int verMajor = 0, verMinor = 0, verBuild = 0;
    int ret = sscanf(version, "%d.%d.%d", &verMajor, &verMinor, &verBuild);
    if (ret == 3) {
        return (verMajor * 1000 + verMinor) * 1000 + verBuild;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// 从服务器取得VER_FILE的版本控制信息，根据此文件进行更新
// VER_FILE文件的格式

//<?xml version="1.0" encoding="gb2312"?>
//<SOFTWARE_INFO>
//  <curversion version="1.3" upgradedate="2002-9-12" obsoletever="1.4.3" TrDays="30">
//    <feature>
//    </feature>
//  </curversion>
//</SOFTWARE_INFO>

int CVersionInfo::fromXML(SXNode *pNodeVer)
{
    if (strcmp(pNodeVer->name.c_str(), SOFTWARE_INFO) != 0)
        return ERR_FALSE;

    // current version
    auto pNode = pNodeVer->getChild("curversion");
    if (!pNode)
        return ERR_FALSE;

    // new version
    verNew = pNode->getPropertySafe("version");
    if (verNew.empty())
        return ERR_FALSE;

    strNewVerDate = pNode->getPropertySafe("upgradedate");

    // new feature
    SXNode *pFeatureNode = pNode->getChild("feature");
    if (pFeatureNode)
        strFeature = pFeatureNode->strContent.c_str();

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////

CVersionUpdate::CVersionUpdate()
{

}

CVersionUpdate::~CVersionUpdate()
{

}

void CVersionUpdate::onDownloadOK(CDownloadTask *pTask)
{
    if (pTask->taskType != DTT_CHECK_VERSION)
        return;

    CVersionInfo    versionInfo;
    int                nRet;

    nRet = getUpdateInfo(pTask, versionInfo);
    if (nRet != ERR_OK)
        return;

    if (VERSION < parseVersionStr(versionInfo.verNew.c_str()))
    {
        string        strMessage = versionInfo.strFeature;
        strMessage += "\r\n\r\n";
        strMessage += _TLT("Do you want to visit our website for more information?");
        if (IDYES == CMPlayerAppBase::getInstance()->messageOut(strMessage.c_str(), MB_YESNO, _TLT("new version Notice")))
        {
            openUrl(CMPlayerAppBase::getMainWnd(), getStrName(SN_HTTP_DOMAIN));
        }
    }
}

int CVersionUpdate::getUpdateInfo(CDownloadTask *pTask, CVersionInfo &versionInfo)
{
    assert(pTask);

    char        szCheckVerTime[256];
    CSimpleXML    xml;
    time_t        lTime;
    tm            *curtime;

    time(&lTime);

    curtime = localtime(&lTime);

    //
    // 将检查更新时间写入配置文件中
    snprintf(szCheckVerTime, CountOf(szCheckVerTime), "%d-%d-%d", curtime->tm_year + 1900, curtime->tm_mon + 1, curtime->tm_mday);
    g_profile.encryptWriteString("CheckVerTime", szCheckVerTime);

    // 分析VER_FILE
    if (!xml.parseData(pTask->buffContent.c_str(), pTask->buffContent.size()))
    {
        ERR_LOG1("parse xml data FAILED: %s", pTask->buffContent.c_str());
        return ERR_PARSE_XML;
    }

    if (versionInfo.fromXML(xml.m_pRoot) != ERR_OK)
        return ERR_PARSE_XML;

    return ERR_OK;
}

void CVersionUpdate::checkNewVersion(bool bAutoCheck)
{
    if (bAutoCheck)
    {
        string strCheckVerTime = g_profile.encryptGetString("CheckVerTime", "");
        CDate dateLastCheck;
        dateLastCheck.fromString(strCheckVerTime.c_str());

        // 如果日期相隔7天，则检查更新
        CDate dateNow = CDate::getCurrentDate();
        if (abs((long)dateNow.getTime() - (long)dateLastCheck.getTime()) <= 7 * CDate::MILLIS_IN_ONE_DAY)
            return;

        g_profile.encryptWriteString("CheckVerTime", dateNow.toUtcDateString().c_str());
    }

    g_LyricsDownloader.downloadVersionFile(getStrName(SN_HTTP_ML_VER), !bAutoCheck);
}

