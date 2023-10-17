#pragma once

#include "ClientCom.h"
#include "HttpProtocol.h"


enum ML_PROTOCAL_ERROR {
    ERR_UNKNOWN                 = 1035, // 未翻译出来的结果
    ERR_BAD_XMLREQUEST          = 1036,
    ERR_CORRUPTTED_MSG          = 1037, // The message is corrupted
    ERR_UPLOAD_EXIST            = 1042, // 上传的内容已经存在
    ERR_USER_NOT_EXIST          = 1043, // 用户不存在
    ERR_BAD_USERPWD             = 1045, // 用户名或者密码错误
    ERR_BAD_CLIENT_VER          = 1047, // 客户端版本不正确, 无法登陆
    ERR_OBSOLETE_CLIENT_VER     = 1048, // 客户端版本太低, 无法登陆
    ERR_ACCESS_DENIED           = 1051, // 无权访问,被拒绝
    ERR_CMD_NOT_MATCH           = 1053, // 命令不匹配
    ERR_CMD_PARAM               = 1054, // 参数错误
    ERR_UPLOAD_FILETYPE         = 1056, // 上传的文件类型错误
    ERR_BAD_BASE64CONTENT       = 1058, // Base64 编码错误
    ERR_BAD_FILE_CONTENT        = 1059, // 文件内容有错误，或者包含不允许的内容
    ERR_NOT_AUTHORIZED          = 1061, // 未经授权的行为
    ERR_NOT_LOGIN               = 1062, // 未登陆，请登陆
    ERR_LYR_NO_TAG_FOUND        = 1064, // No lyrics tag is found in content.
    ERR_DATABASE_ERROR          = 1065, // 内部数据库错误
};

#define SERVER_PORT_BEGIN   8000
#ifdef _IPHONE
#define SERVER_PORT_END     8010
#else
#define SERVER_PORT_END     8050
#endif

#define ML_SERVER_PORT      1212


enum MLMsgType {
    MT_UNKNOWN,                      // 未知格式
    MT_XML,                          // xml的消息格式

    MT_HTTP_GET,                     // Http get request
    MT_HTTP_POST,                    // Http post request
    MT_HTTP_RET,                     // Http return

    MT_HTTP_XML,                     // Minilyrics 请求的HTTP tunnel的消息格式,
    // translated from MT_HTTP_*
    MT_HTTP_BIN_XML,                 // Similar with MT_HTTP_XML, but it's binary xml format.
    // See CMLBinXMLParser
};

//
// Command send from client to server
//
enum MLMsgCmd {
    MC_UNKNOWN,                      // Unknown
    MC_LOGIN,                        // log in
    MC_SEARCH_V0,                    // Old version of search lyrics command. ML 7.1 before
    MC_SEARCH,                       // search lyrics
    MC_BATCH_SEARCH,                 // search lyrics by batch
    MC_CAN_UPLOAD,                   // Deprecated command. return ERR_NOT_FOUND directly.
    MC_UPLOADV0,                     // Old version of upload lyrics.
    MC_UPLOAD,                       // 7.1 upload lyrics command
    MC_ACTIVATE,
};

MLMsgCmd mLMsgStr2Cmd(cstr_t szMLMsgCmd);


#define MZ_LOGIN            "login"
#define MZ_RETURN           "return"
#define MZ_SEARCH_V0        "search"
#define MZ_SEARCH           "searchV1"
#define MZ_BATCH_SEARCH     "batchsearch"        // MiniLyrics 7.1 or later
#define MZ_CANUPLOAD        "canupload"
#define MZ_UPLOADV0         "upload"
#define MZ_UPLOAD           "uploadV1"

#define MZ_GET_LYR_UPDATE   "GetLyrUpdate"
#define MZ_SUBMIT_LYR_REVIEW "SubmitLyrReview"
#define MZ_EXE_SVR_CMD      "ExeSvrCmd"

#define MZ_FILEINFO         "fileinfo"
#define MZ_ITEM             "item"
#define MZ_ORG_CMD          "orgcmd"
#define MZ_CMD_RESULT       "result"


#define PRODUCT_ID_INVALID  0
#define PRODUCT_ID_MINILYRICS   1

class CXMLMsgCmd {
public:
    CXMLMsgCmd() { nProductId = PRODUCT_ID_INVALID; }
    virtual ~CXMLMsgCmd() { }

    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile) = 0;

    virtual void toXMLAttribute(CXMLWriter &xmlStream);

public:
    string                      strClient;          // MiniLyrics for Winamp2 7.0.xxx
    int                         nProductId;         // See PRODUCT_ID_MINILYRICS

};

// Returned message
class MLRetMsg {
public:
    string                      strOrgCmd;          // original command
    string                      strMessage;         // Message to client
    int                         result;

public:
    MLRetMsg() { }
    virtual ~MLRetMsg() { }

    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

    virtual void toXMLAttribute(CXMLWriter &xmlFile);

};

//////////////////////////////////////////////////////////////////////////
//
// login command
//
class MLMsgCmdLogin : public CXMLMsgCmd {
public:
    string                      name;
    string                      strPwd;

public:
    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

// Return of log in command
typedef MLRetMsg MLMsgRetLogin;

// Returned search result lyrics info
class RetLyrInfo {
public:
    string                      strArtist;
    string                      strTitle;
    string                      strAlbum;
    string                      strUploader;
    string                      strLink;
    int                         nTimeLength;        // Media length in second
    float                       fRate;              // rate info of lyrics
    int                         nRateCount;         //
    int                         nDownloads;         // total downloads of lyrics

    // For internal usage, will NOT send to client.
    int                         uploaderID, lyricsID, contentType;

public:
    RetLyrInfo() {
        nTimeLength = 0;
        fRate = 0.0;
        nRateCount = 0;
        nDownloads = 0;
    }

    virtual int fromXML(SXNode *pNode, cstr_t szServerUrl);
    virtual int toXML(CXMLWriter &xmlFile, string &strServerUrl, bool bV0Protocol);

    string getFileName();

};

typedef list<RetLyrInfo> RetLyrInfoList;

enum ML_EDITION {
    ME_NOTSET                   = 0,
    ME_ENGLISH                  = 1,
    ME_CHINESE                  = 2,
};

// search lyrics command
class MLMsgCmdSearch : public CXMLMsgCmd {
public:
    string                      strArtist;
    string                      strTitle;
    int                         nRequestPage;
    bool                        bOnlyMatchedLyrics; // Return only matched lyrics

public:
    MLMsgCmdSearch() {
        nRequestPage = 0;
        bOnlyMatchedLyrics = false;
    }
    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

// Return of search lyrics command
class MLMsgRetSearch : public MLRetMsg {
public:
    int                         nPageCount, nCurPage;

    RetLyrInfoList              listResultFiles;

    // For 7.1 above version, RetLyrInfo.strLink including full URL address
    // After 7.1 only include related path, base URL is saved in strServerUrl.
    bool                        bV0Protocol;        // 7.1 before version of Protocol?
    string                      strServerUrl;

public:
    MLMsgRetSearch() {
        nPageCount = 1;
        nCurPage = 0;
        bV0Protocol = false;
    }
    virtual int fromXML(SXNode *pNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

//////////////////////////////////////////////////////////////////////////
//
// Batch lyrics search item
class MLSearchItem {
public:
    MLSearchItem() : nMediaLength(0) { }

    int fromXML(SXNode *pXmlNode);
    int toXML(CXMLWriter &xmlFile);

    string                      strArtist, strAlbum, strTitle;
    int                         nMediaLength;       // in seconds

    // Fields below are used on in CLIENT of MiniLyrics.
    string                      strMediaFile;
    int                         nIndex;

};
typedef vector<MLSearchItem> MLListSearchItems;

// Client sent to server to search lyrics by batch.
class MLMsgCmdBatchSearch : public CXMLMsgCmd {
public:
    MLListSearchItems           listSearchItems;
    bool                        bOnlyActiveCanDownload;

public:
    MLMsgCmdBatchSearch();

    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

// Returned lyrics info for batch lyrics search.
class MLLyricsInfoLite {
public:
    // For client side, strFile == strSaveName.
    // For server side, strFile is the file path name of lyrics, strSaveName is the file name to save.
    string                      strFile, strSaveName;
    string                      bufLyrContent;

public:
    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

typedef list<MLLyricsInfoLite> ListLyricsInfoLite;

// The return message of batch lyrics search
class MLMsgRetBatchSearch : public MLRetMsg {
public:
    ListLyricsInfoLite          listLyricsInfo;     // Lyrics info: artist, title, file name

public:
    virtual int fromXML(SXNode *pXmlNode);
    virtual int toXML(CXMLWriter &xmlFile);

};


// Old lyrics upload command
//  7.0 and before.
class MLMsgCmdUploadV0 : public CXMLMsgCmd {
public:
    string                      strLoginName;
    string                      strFileContent;     // Uploaded lyrics content, encoded in base64.

public:
    virtual int fromXML(SXNode *pNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

// upload lyrics command 7.1 or later
class MLMsgCmdUpload : public CXMLMsgCmd {
public:
    string                      strLoginName;
    string                      strPwdMask;
    string                      strFileContent;     // FileContentMust be in UTF-8 encoding, NO BOM.

public:
    virtual int fromXML(SXNode *pNode);
    virtual int toXML(CXMLWriter &xmlFile);

};

// new version of upload lyrics command (since 7.1 or later)
class MLMsgRetUpload : public MLRetMsg {
public:
    string                      strLyricsId;  // Returned lyrics ID, to be saved in lyrics file.

public:
    virtual int fromXML(SXNode *pNode);
    virtual void toXMLAttribute(CXMLWriter &xmlFile);

};

// Old xml message (3.0)
#define ML_MSG_MAX_LENGTH        (1024 * 1024)
#define ML_MSG_XML          "XML"
#define ML_MSG_XML_FMT      "XML%6d"
#define ML_MSG_XML_LEN      3
#define ML_MSG_XML_DATA_LEN 6


enum ML_PACK_VERSION {
    MPV_INVALID                 = 0,
    MPV_XML                     = '<', // 没有加密，直接为 XML 协议
    MPV_V1_MD5_ID               = 2,
};

#pragma pack(push)
#pragma pack(1)
struct MLEncodePacketV1 {
    uint8_t                     byVersion;          // 协议的版本：
    uint8_t                     byXor;              // XOR 解密的字段, byDataOrg 的平均值 ^ 0x9D
    uint8_t                     byID[4];            // 客户端使用的MD5验证码ID
    // nID = byID[0] | byID[1] << 8 | byID[2] << 16 | byID[3] << 24
    uint8_t                     byMd5[16];          // 校验 消息包正确性的MD5值

    void setID(long nId) {
        byID[0] = (uint8_t)(nId & 0xFF);
        byID[1] = (uint8_t)((nId >> 8) & 0xFF);
        byID[2] = (uint8_t)((nId >> 16) & 0xFF);
        byID[3] = (uint8_t)((nId >> 24) & 0xFF);
    }

    long getID() {
        return byID[0] | (byID[1] << 8) | (byID[2] << 16) | (byID[3] << 24);
    }
};
#pragma pack(pop)

class CMLPacketAccountMgr {
public:
    static CMLPacketAccountMgr *getInstance();

    void addAccount(long nId, const char*szPwd);
    void eraseAccount(long nId);

    const char* GetPwd(long nAccountId);

    void setMode(bool bServer, ML_PACK_VERSION packVer) {
        m_bServer = bServer;
        m_packFlag = packVer;
    }

protected:
    CMLPacketAccountMgr();

    typedef map<long, string>    MAP_ACCOUNT;

    MAP_ACCOUNT                 m_mapAccount;

    static CMLPacketAccountMgr  *m_pMLPacketWrapper;

public:
    ML_PACK_VERSION             m_packFlag;
    bool                        m_bEnableXmlPack;   // 是否允许未加密的 XML packet
    bool                        m_bServer;          // 是否为服务器端

    // 统计数据
    long                        m_nInvalidV1PackCount; // 无效的V1 packet 数目
    long                        m_nInvalidPackCount; // 无效的 packet 数目

};

class CMLPacketWrapper {
public:
    CMLPacketWrapper();

    int wrapp(string &buff);
    int unwrapp(string &buff);

    void setId(int nId)
        { m_nCurrentId = nId; }

    ML_PACK_VERSION getPackVersion() { return m_packFlag; }

protected:
    long                        m_nCurrentId;
    CMLPacketAccountMgr         *m_pAccountMgr;
    ML_PACK_VERSION             m_packFlag;

};

class CMLProtocol {
public:
    CMLProtocol();
    virtual ~CMLProtocol();

    int init(CNetFile *pFile, cstr_t szHost, int nPort);

    int setNetFile(CNetFile *pFile) { m_pNetFile = pFile; return ERR_OK; }

    void setProxy(bool bEnableProxy, cstr_t szProxyServer, int nProxyPort, cstr_t szBase64ProxyUserPass = nullptr);

    int readMsg(MLMsgType &msgType, string *pMsgBuff);

    int readOldXmlMsg(MLMsgType &msgType, string *pMsgBuff);
    int writeOldXmlMsg(string *pMsgBuff);

    int writeXMLMsg(MLMsgType msgType, string *pMsgBuff);

    int parseXMLMsg(string *pMsgBuff, CSimpleXML &xml);

    static MLMsgCmd getMsgCommand(CSimpleXML &xml);

    void setClientId(int nId) {
        m_packetWrapper.setId(nId);
    }

protected:
    //
    // proxy
    bool                        m_bUseProxy;
    string                      m_strBase64ProxyUserPass;

    // 为了提高效率，而设置的缓存区
    CHttpRequestProtocol        m_httpReqWriteXmlCache;
    CHttpReturnProtocol         m_httpRetWriteXmlCache;

    CNetFile                    *m_pNetFile;

public:
    CHttpRequestProtocol        m_httpRequest;
    CHttpReturnProtocol         m_httpReturn;

    CMLPacketWrapper            m_packetWrapper;

};

class CMLServerProtocol : public CMLProtocol {
public:

};
