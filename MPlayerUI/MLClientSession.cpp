#include "MLClientSession.h"
#include "MLProfile.h"


#define CUR_ACCOUNT_ID      4
#define CUR_ACCOUNT_NAME    "Mlv1clt4.0"
#define CUR_MSG_TYPE        MT_HTTP_POST

ServerCfg        g_LyrServers[SERVER_COUNT] = {
    { "", 80 },
};

extern CProfile g_profile;

int g_nCurLyrServer = 0;
int g_nPortSearchServer = 80;
int g_nPortDownloadServer = 80;

void loadServerInfo() {
    string strEng;

    if (isEmptyString(g_LyrServers[0].szServer)) {
        int nEngIndex = 0;
        g_LyrServers[nEngIndex].nPort = 80;
        strcpy_safe(g_LyrServers[nEngIndex].szServer,
            CountOf(g_LyrServers[nEngIndex].szServer), "search.crintsoft.com");
    }
}

//////////////////////////////////////////////////////////////////////

CMLClientSession::CMLClientSession() {
    m_bLogined = false;
    m_bEnableProxy = false;
    m_nProxyPort = 80;
    m_bInitialzed = false;
}

CMLClientSession::~CMLClientSession() {

}

void CMLClientSession::init(cstr_t szAppName) {
    if (m_bInitialzed) {
        return;
    }
    m_bInitialzed = true;

    m_strClient = szAppName;
    loadServerInfo();

    m_strLoginName = g_profile.getString("LoginName", "");
    m_strPwdMask = g_profile.getString("LoginPwdMask", "");

    m_MsgProtocol.init(&m_NetFile, getDefaultServer(), getDefaultServerPort());

    CMLPacketAccountMgr *pMgr = CMLPacketAccountMgr::getInstance();
    pMgr->addAccount(CUR_ACCOUNT_ID, CUR_ACCOUNT_NAME);
    m_MsgProtocol.setClientId(CUR_ACCOUNT_ID);
    pMgr->setMode(false, MPV_V1_MD5_ID);

}

int CMLClientSession::connect() {
    int nRet;

    m_bClosing = false;

    if (m_ClientCom.isValid()) {
        m_ClientCom.close();
    }

    int nProxyPort;
    string proxyServer;
    bool bUserProxy = CMLProfile::inetGetProxy(proxyServer, nProxyPort);
    string strProxyPwd = CMLProfile::inetGetBase64ProxyUserPass();
    setProxy(bUserProxy, proxyServer.c_str(), nProxyPort, strProxyPwd.c_str());

    if (m_bEnableProxy) {
        nRet = m_ClientCom.connect(m_proxyServer.c_str(), m_nProxyPort);
    } else {
        // Try to connect to server: Latest port, 80, 8000 ~ 8080
        nRet = m_ClientCom.connect(getDefaultServer(), g_nPortSearchServer);
        if (nRet != ERR_OK && !m_bClosing) {
            // 80
            if (g_nPortSearchServer != 80) {
                nRet = m_ClientCom.connect(getDefaultServer(), 80);
            }
            if (nRet == ERR_OK) {
                g_nPortSearchServer = 80;
            } else {
                // 8000 ~ 8080
                if (g_nPortSearchServer < SERVER_PORT_BEGIN) {
                    g_nPortSearchServer = SERVER_PORT_BEGIN - 1;
                }
                for (int i = SERVER_PORT_BEGIN; !m_bClosing && i <= SERVER_PORT_END; i++) {
                    g_nPortSearchServer++;
                    if (g_nPortSearchServer > SERVER_PORT_END) {
                        g_nPortSearchServer = SERVER_PORT_BEGIN;
                    }
                    nRet = m_ClientCom.connect(getDefaultServer(), g_nPortSearchServer);
                    if (nRet == ERR_OK || nRet == ERR_NET_HOST_NOT_FOUND) {
                        break;
                    }
                }
            }
        }
    }
    if (nRet != ERR_OK) {
        return nRet;
    }

    m_NetFile.attach(m_ClientCom.getSocket());
    m_NetFile.setTimeOut(20 * 1000);

    /*    nRet = m_MsgProtocol.init(&m_NetFile);
    if (nRet != ERR_OK)
        return nRet;*/

    return ERR_OK;
}

void CMLClientSession::setUploader(cstr_t szUser, cstr_t szPwd, bool bSavePwd) {
    if (strcmp(szPwd, m_strPwdMask.c_str()) != 0
        || strcmp(szUser, m_strLoginName.c_str()) != 0) {
        m_strPwdMask = md5ToString16(szPwd);;
        m_strLoginName = szUser;
    }

    cstr_t szPwdSaved = g_profile.getString("LoginPwdMask", "");
    cstr_t szPwdToSave;
    if (bSavePwd) {
        szPwdToSave = m_strPwdMask.c_str();
    } else {
        szPwdToSave = "";
    }

    if (strcmp(szPwdToSave, szPwdSaved) != 0) {
        g_profile.writeString("LoginPwdMask", szPwdToSave);
    }

    if (strcmp(szPwdToSave, g_profile.getString("LoginName", "")) != 0) {
        g_profile.writeString("LoginName", szUser);
    }
}

void CMLClientSession::setProxy(bool bEnableProxy, cstr_t szProxyServer, int nPort, cstr_t szBase64ProxyUserPass) {
    if (bEnableProxy && !isEmptyString(szProxyServer)) {
        m_proxyServer = szProxyServer;
        m_nProxyPort = nPort;
        m_bEnableProxy = true;
    } else {
        m_bEnableProxy = false;
    }

    m_MsgProtocol.setProxy(bEnableProxy, szProxyServer, nPort, szBase64ProxyUserPass);
}

int CMLClientSession::login(string &strMsgRet) {
    MLMsgCmdLogin cmdLogin;
    int nRet;

    assert(m_strLoginName.size() && m_strPwdMask.size());
    cmdLogin.name = m_strLoginName;
    cmdLogin.strPwd = m_strPwdMask;
    prepareClientInfo(cmdLogin);

    m_bLogined = false;
    nRet = sendXmlCommand(cmdLogin);
    if (nRet != ERR_OK) {
        return nRet;
    }

    MLMsgRetLogin retLogin;
    nRet = processCommonRetMsg(&retLogin, MC_LOGIN);

    if (nRet == ERR_OK) {
        m_bLogined = true;
    }

    strMsgRet = retLogin.strMessage;

    return nRet;
}

// 返回值：
//    ERR_NOT_FOUND
//
int CMLClientSession::search(bool bOnlyMatched, cstr_t szArtist, cstr_t szTitle, int nRequestPage, MLMsgRetSearch &retSearch) {
    int nRet;
    MLMsgCmdSearch cmdSearch;

    retSearch.strMessage = "";
    cmdSearch.strTitle = szTitle;
    cmdSearch.strArtist = szArtist;
    cmdSearch.nRequestPage = nRequestPage;
    cmdSearch.bOnlyMatchedLyrics = bOnlyMatched;
    prepareClientInfo(cmdSearch);

    nRet = sendXmlCommand(cmdSearch);
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = processCommonRetMsg(&retSearch, MC_SEARCH);

    if (nRet != ERR_OK) {
        return nRet;
    }

    return ERR_OK;
}

int CMLClientSession::batchSearch(MLListSearchItems &listToSearch, MLMsgRetBatchSearch &retSearch) {
    int nRet;
    MLMsgCmdBatchSearch cmdSearch;

    retSearch.strMessage = "";

    cmdSearch.bOnlyActiveCanDownload = true;
    cmdSearch.listSearchItems = listToSearch;
    prepareClientInfo(cmdSearch);

    nRet = sendXmlCommand(cmdSearch);
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = processCommonRetMsg(&retSearch, MC_BATCH_SEARCH);

    return nRet;
}

int CMLClientSession::upload(const string &lyricsContent, string &strLyricsId, string &strMsg) {
    MLMsgCmdUpload cmdUpload;

    assert(m_strLoginName.size() && m_strPwdMask.size());
    cmdUpload.strLoginName = m_strLoginName;
    cmdUpload.strPwdMask = m_strPwdMask;
    cmdUpload.strFileContent = lyricsContent;

    int nRet = sendXmlCommand(cmdUpload);
    if (nRet != ERR_OK) {
        return nRet;
    }

    MLMsgRetUpload retUpload;

    nRet = processCommonRetMsg(&retUpload, MC_UPLOAD);
    if (nRet == ERR_BAD_USERPWD) {
        m_bLogined = false;
    }

    strLyricsId = retUpload.strLyricsId;
    strMsg = retUpload.strMessage;

    return nRet;
}

void CMLClientSession::prepareClientInfo(CXMLMsgCmd &cmd) {
    cmd.strClient = m_strClient;
    cmd.nProductId = PRODUCT_ID_MINILYRICS;
}

int CMLClientSession::sendXmlCommand(CXMLMsgCmd &cmd) {
    // CXMLWriter        xmlStream;
    CMLBinXMLWriter xmlStream;

    int nRet = cmd.toXML(xmlStream);
    if (nRet != ERR_OK) {
        return nRet;
    }

    return m_MsgProtocol.writeXMLMsg(CUR_MSG_TYPE, &xmlStream.getBuffer());
}

int CMLClientSession::processCommonRetMsg(MLRetMsg *pretMsg, MLMsgCmd cmdOrgSend) {
    assert(pretMsg);
    MLMsgType msgType;
    string buffMsg;
    int nRet;
    buffMsg.reserve(2048);
    nRet = m_MsgProtocol.readMsg(msgType, &buffMsg);
    if (nRet != ERR_OK) {
        DBG_LOG0("READ MSG FAILED!");
        return nRet;
    }

    CSimpleXML xml;
    nRet = m_MsgProtocol.parseXMLMsg(&buffMsg, xml);
    if (nRet != ERR_OK) {
        return nRet;
    }

    if (strcmp(xml.m_pRoot->name.c_str(), MZ_RETURN) != 0) {
        return ERR_CMD_NOT_MATCH;
    }

    nRet = pretMsg->fromXML(xml.m_pRoot);
    if (nRet != ERR_OK) {
        return nRet;
    }

    MLMsgCmd cmdOrg = mLMsgStr2Cmd(pretMsg->strOrgCmd.c_str());
    if (cmdOrg != cmdOrgSend) {
        return ERR_CMD_NOT_MATCH;
    }

    return pretMsg->result;
}

/*

//////////////////////////////////////////////////////////////////////////
// 所有函数的返回结果说明：
// 返回0，表示该函数调用成功；
// 返回其他值，表示该函数调用失败；使用getErrorDesc，可以取得具体的错误描述
//

#ifndef ERR_OK
#define ERR_OK        0
#endif

struct LyricsInfo
{
    char szFileName[MAX_PATH];
    char szArtist[MAX_PATH];
    char szTitle[MAX_PATH];
};

extern "C"
{

// 登陆
int login(cstr_t szName, cstr_t szPwd);

// 搜索，传入文件名，返回搜索结果句柄，和对应的结果个数
// 如果没有歌词，仍然返回成功
int searchLyrics(cstr_t szFileName, HANDLE &hResult, int &nCount);

// 根据结果索引，返回每一个结果的具体信息
int getLyricsInfo(HANDLE hLyrics, int nIndex, LyricsInfo &lyricsInfo);

// 在不使用结果句柄之后，必须调用此函数，以释放内存
int freeLyricsHandle(HANDLE hLyrics);

// 下载歌词, 
// 参数：
//        szDownloadDir    : 歌词下载到的目录
//        szLyricsFile    ：下载之后的文件名，包括路径
int downloadLyrics(HANDLE hLyrics, int nIndex, cstr_t szDownloadDir, char * szLyricsFile);

// 根据返回的错误ID，返回错误描述
cstr_t getErrorDesc(int nRet);

}
//
// example:

// 初始化
int init()
{
    int        nRet;

    // 登陆
    nRet = login("name", "password")
    if (nRet != ERR_OK)
    {
        CString        str;
        str.Format("登陆失败，原因: %s", getErrorDesc(nRet));
        AfxMessageBox(str);
    }
}

int yourSearchLyrics(cstr_t szFile)
{
    HANDLE    hResult;
    int        nRet, nCount;
    CString        str;

    nRet = searchLyrics(szFile, hResult, nCount);
    if (nRet != ERR_OK)
    {
        str.Format("搜索失败，原因: %s", getErrorDesc(nRet));
        AfxMessageBox(str);
        return 0;
    }

    for (int i = 0; i < nCount; i++)
    {
        LyricsInfo        lyricsInfo;
        nRet = getLyricsInfo(hResult, i, lyricsInfo);
        if (nRet == ERR_OK)
        {
            // 显示这些信息，添加到列表中，由用户选择
            //....
        }
    }

    char        szLyricsFile[MAX_PATH];
    char        szSaveDir[MAX_PATH] = "C:\\Download";

    // 选择下载第一首
    nRet = downloadLyrics(hResult, 0, szSaveDir, szLyricsFile);
    if (nRet != ERR_OK)
    {
        str.Format("下载失败，原因: %s", getErrorDesc(nRet));
        AfxMessageBox(str);
        return 0;
    }

    // 释放内存
    freeLyricsHandle(hResult);

}

 */
