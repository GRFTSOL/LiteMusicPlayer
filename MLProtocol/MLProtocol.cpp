#include "MLProtocol.h"


/***********************************************************************
History of MLProtocol:

ML 3.0:            Old XML message format, encoding: gb2312(system default).
                disabled now

ML 3.1 ~ 3.9:    HTTP tunnel XML message format, encoding: gb2312(system default).
                Deprecated: Protocol version: "0.9", disabled now

ML 4.0 ~ 7.0:    HTTP tunnel Encrypted XML message format, encoding: UTF-8

ML 7.1 ~ :        HTTP tunnel Encrypted Binary XML message format, encoding: UTF-8
2011.03            support batch download, ML Binary XML message format

\***********************************************************************/

#define SZ_ML_POST_URL      "/searchlyrics.htm"

#define SZ_DATA_BLOCK_SIGN  "\0$"
#define LEN_DATA_BLOCK_SIGN 2

static IdToString    __ErrID2Str[] = {
    { ERR_UNKNOWN, "Unknown Error."},
    { ERR_BAD_XMLREQUEST, "Invalid xml request."},
    { ERR_CORRUPTTED_MSG, "The message is corrupted, maybe some data get currupted while receiving."},

    { ERR_UPLOAD_EXIST, "The uploaded lyrics already exist, or other users have uploaded them already." },
    { ERR_USER_NOT_EXIST, "User name doesn't exist." },
    { ERR_BAD_USERPWD, "Wrong user name or password." },
    { ERR_BAD_CLIENT_VER, "Invliad client version." },
    { ERR_OBSOLETE_CLIENT_VER, "Obsoleted client. please download the latest version." },
    // { ERR_TOO_BUSY, "Server too busy, please try again later." },
    { ERR_ACCESS_DENIED, "Access denied." },
    { ERR_CMD_NOT_MATCH, "Received return command not match." },
    { ERR_CMD_PARAM, "Invalid message command parameter." },
    { ERR_UPLOAD_FILETYPE, "The file type can not be uploaded." },
    { ERR_BAD_BASE64CONTENT, "Incorrect base64 encoded content." },
    { ERR_BAD_FILE_CONTENT, "The file format is incorrect or contains spam information." },
    { ERR_NOT_AUTHORIZED, "Unauthorized action." },
    { ERR_NOT_LOGIN, "Not signed in, please sign in." },
    { ERR_LYR_NO_TAG_FOUND, "Please fill in lyrics artist and title information." },
    { ERR_DATABASE_ERROR, "Internal database error, please contact us to report this error." },

    { 0, nullptr}
};

static RegErr2Str __addErr2Str(__ErrID2Str);

IdToString    __MlCmdIDStr[] = {
    { MC_LOGIN, MZ_LOGIN },
    { MC_SEARCH, MZ_SEARCH },
    { MC_BATCH_SEARCH, MZ_BATCH_SEARCH },
    { MC_CAN_UPLOAD, MZ_CANUPLOAD},
    { MC_SEARCH_V0, MZ_SEARCH_V0 },
    { MC_UPLOADV0, MZ_UPLOADV0 },
    { MC_UPLOAD, MZ_UPLOAD },
    { MC_UNKNOWN, nullptr},
};

MLMsgCmd mLMsgStr2Cmd(cstr_t szMLMsgCmd) {
    return (MLMsgCmd)stringToID(__MlCmdIDStr, szMLMsgCmd, MC_UNKNOWN);
}

cstr_t mLMsgCmd2Str(MLMsgCmd msgCmd) {
    return iDToString(__MlCmdIDStr, msgCmd, "Not found");
}

IdToString    __MlCmdResult2Str[] = {
    { ERR_OK, "OK" },
    { ERR_FAILED, "FAILED" },
    { ERR_BAD_XMLREQUEST, "BAD_XML_REQUEST" },
    { ERR_UPLOAD_EXIST, "UPLOAD_EXIST" },
    { ERR_USER_NOT_EXIST, "USER_NOT_FOUND" },
    { ERR_BAD_USERPWD, "BAD_USERPWD" },
    { ERR_BAD_CLIENT_VER, "BAD_CLIENT_VER" },
    { ERR_OBSOLETE_CLIENT_VER, "OBSOLETE_CLIENT_VER" },
    { ERR_NOT_FOUND, "NOT_FOUND" },
    { ERR_ACCESS_DENIED, "ACCESS_DENIED" },
    { ERR_CMD_NOT_MATCH, "CMD_NOT_MATCH" },
    { ERR_CMD_PARAM, "BAD_CMD_PARAM" },
    { ERR_UPLOAD_FILETYPE, "UPLOAD_FILETYPE" },
    { ERR_BAD_BASE64CONTENT, "BAD_BASE64CONTENT" },
    { ERR_BAD_FILE_CONTENT, "BAD_FILE_CONTENT" },
    { ERR_NOT_AUTHORIZED, "NOT_AUTHORIZED" },
    { ERR_NOT_LOGIN, "NOT_LOGIN" },
    { ERR_LYR_NO_TAG_FOUND, "LYR_NO_TAG_FOUND" },
    {0, nullptr},
};

int CXMLMsgCmd::fromXML(SXNode *pXmlNode) {
    strClient = pXmlNode->getPropertySafe("client");
    nProductId = pXmlNode->getPropertyInt("pid", PRODUCT_ID_MINILYRICS);
    assert(nProductId != PRODUCT_ID_INVALID);

    return ERR_OK;
}

void CXMLMsgCmd::toXMLAttribute(CXMLWriter &xmlStream) {
    xmlStream.writeAttribute("client", strClient.c_str());
    if (nProductId != PRODUCT_ID_MINILYRICS) {
        xmlStream.writeAttribute("pid", nProductId);
    }
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdLogin::fromXML(SXNode *pXmlNode) {
    CXMLMsgCmd::fromXML(pXmlNode);

    name = pXmlNode->getPropertySafe("user");
    strPwd = pXmlNode->getPropertySafe("password");

    return ERR_OK;
}

int MLMsgCmdLogin::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_LOGIN);
    xmlFile.writeAttribute("user", name.c_str());
    xmlFile.writeAttribute("password", strPwd.c_str());
    CXMLMsgCmd::toXMLAttribute(xmlFile);
    xmlFile.writeEndElement();
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
int MLRetMsg::fromXML(SXNode *pXmlNode) {
    strOrgCmd = pXmlNode->getPropertySafe(MZ_ORG_CMD);
    cstr_t szValue = pXmlNode->getProperty(MZ_CMD_RESULT);
    if (!szValue) {
        return ERR_CMD_PARAM;
    }

    result = stringToID(__MlCmdResult2Str, szValue, ERR_UNKNOWN);

    strMessage = pXmlNode->getPropertySafe("msg");
    if (strMessage.empty()) {
        strMessage = pXmlNode->getPropertySafe("message");
    }

    return ERR_OK;
}

int MLRetMsg::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_RETURN);
    toXMLAttribute(xmlFile);
    xmlFile.writeEndElement();
    return ERR_OK;
}

void MLRetMsg::toXMLAttribute(CXMLWriter &xmlFile) {
    xmlFile.writeAttribute(MZ_ORG_CMD, strOrgCmd.c_str());
    xmlFile.writeAttribute(MZ_CMD_RESULT, iDToString(__MlCmdResult2Str, result, "UNTRANSLATED"));

    if (strMessage.size()) {
        xmlFile.writeAttribute("msg", strMessage.c_str()); // msg is for search and upload
        xmlFile.writeAttribute("message", strMessage.c_str()); // message is for login
    }
}

//////////////////////////////////////////////////////////////////////////

int RetLyrInfo::fromXML(SXNode *pNode, cstr_t szServerUrl) {
    assert(szServerUrl);

    strArtist = pNode->getPropertySafe("artist");
    strTitle = pNode->getPropertySafe("title");
    strAlbum = pNode->getPropertySafe("album");
    strUploader = pNode->getPropertySafe("uploader");

    strLink = szServerUrl;
    strLink += pNode->getPropertySafe("link");

    nTimeLength = pNode->getPropertyInt("timelength");

    cstr_t szValue = pNode->getProperty("rate");
    if (szValue) {
        sscanf(szValue, "%f", &fRate);
    }
    nRateCount = pNode->getPropertyInt("ratecount");
    nDownloads = pNode->getPropertyInt("downloads");

    return ERR_OK;
}

int RetLyrInfo::toXML(CXMLWriter &xmlFile, string &strServerUrl, bool bV0Protocol) {
    xmlFile.writeStartElement(MZ_FILEINFO);

    if (bV0Protocol) {
        xmlFile.writeAttribute("filetype", "lyrics"); // This must be set for old version (7.0.614 before).
        xmlFile.writeAttribute("link", (strServerUrl + strLink).c_str());

        xmlFile.writeAttribute("filename", getFileName().c_str());
    } else {
        xmlFile.writeAttribute("link", strLink.c_str());
    }

    if (!strArtist.empty()) {
        xmlFile.writeAttribute("artist", strArtist.c_str());
    }
    if (!strTitle.empty()) {
        xmlFile.writeAttribute("title", strTitle.c_str());
    }
    if (!strAlbum.empty()) {
        xmlFile.writeAttribute("album", strAlbum.c_str());
    }
    if (!strUploader.empty()) {
        xmlFile.writeAttribute("uploader", strUploader.c_str());
    }
    if (nTimeLength != 0) {
        xmlFile.writeAttribute("timelength", nTimeLength);
    }
    if (fRate != 0.0) {
        char szTemp[64];
        snprintf(szTemp, CountOf(szTemp), "%.1f", fRate);
        xmlFile.writeAttribute("rate", szTemp);
    }
    if (nRateCount != 0) {
        xmlFile.writeAttribute("ratecount", nRateCount);
    }
    if (nDownloads != 0) {
        xmlFile.writeAttribute("downloads", nDownloads);
    }
    xmlFile.writeEndElement();
    return ERR_OK;
}

string RetLyrInfo::getFileName() {
    assert(strTitle.size());

    // Combine file name
    string strFileName;
    strFileName = strArtist;
    if (strFileName.size()) {
        strFileName += " - ";
    }
    strFileName += strTitle;
    strFileName = fileNameFilterInvalidChars(strFileName.c_str());
    strFileName += fileGetExt(strLink.c_str());

    return strFileName;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdSearch::fromXML(SXNode *pXmlNode) {
    CXMLMsgCmd::fromXML(pXmlNode);

    strArtist = pXmlNode->getPropertySafe("artist");
    strTitle = pXmlNode->getPropertySafe("title");
    nRequestPage = pXmlNode->getPropertyInt("RequestPage");
    bOnlyMatchedLyrics = tobool(pXmlNode->getPropertyInt("OnlyMatched"));

    if (strArtist.empty() && strTitle.empty()) {
        return ERR_CMD_PARAM;
    }

    return ERR_OK;
}

int MLMsgCmdSearch::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_SEARCH);

    CXMLMsgCmd::toXMLAttribute(xmlFile);

    if (strArtist.size()) {
        xmlFile.writeAttribute("artist", strArtist.c_str());
    }
    if (strTitle.size()) {
        xmlFile.writeAttribute("title", strTitle.c_str());
    }
    if (nRequestPage != 0) {
        xmlFile.writeAttribute("RequestPage", nRequestPage);
    }

    if (bOnlyMatchedLyrics) {
        xmlFile.writeAttribute("OnlyMatched", bOnlyMatchedLyrics);
    }

    xmlFile.writeEndElement();
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgRetSearch::fromXML(SXNode *pNode) {
    int nRet = MLRetMsg::fromXML(pNode);
    if (nRet != ERR_OK) {
        return nRet;
    }

    assert(!bV0Protocol);

    strServerUrl = pNode->getPropertySafe("server_url");
    nPageCount = pNode->getPropertyInt("PageCount");
    nCurPage = pNode->getPropertyInt("CurPage");

    SXNode::iterator it, itEnd;

    itEnd = pNode->listChildren.end();
    for (it = pNode->listChildren.begin(); it != itEnd; ++it) {
        RetLyrInfo Info;
        Info.fromXML(*it, strServerUrl.c_str());
        listResultFiles.push_back(Info);
    }

    return ERR_OK;
}

int MLMsgRetSearch::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_RETURN);

    toXMLAttribute(xmlFile);

    if (bV0Protocol) {
        xmlFile.writeAttribute("filetype", "lyrics"); // This should be set for old version (7.0.614 before).
    } else if (strServerUrl.size() > 0) {
        xmlFile.writeAttribute("server_url", strServerUrl.c_str());
    }

    if (nPageCount != 1) {
        xmlFile.writeAttribute("PageCount", nPageCount);
    }

    if (nCurPage != 0) {
        xmlFile.writeAttribute("CurPage", nCurPage);
    }

    for (list<RetLyrInfo>::iterator it = listResultFiles.begin(); it != listResultFiles.end(); ++it) {
        (*it).toXML(xmlFile, strServerUrl, bV0Protocol);
    }

    xmlFile.writeEndElement();

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////

int MLSearchItem::fromXML(SXNode *pXmlNode) {
    strArtist = pXmlNode->getPropertySafe("ar");
    strAlbum = pXmlNode->getPropertySafe("al");
    strTitle = pXmlNode->getPropertySafe("ti");

    nMediaLength = pXmlNode->getPropertyInt("length");

    return ERR_OK;
}

int MLSearchItem::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_ITEM);

    if (strArtist.size()) {
        xmlFile.writeAttribute("ar", strArtist.c_str());
    }
    if (strAlbum.size()) {
        xmlFile.writeAttribute("al", strAlbum.c_str());
    }
    xmlFile.writeAttribute("ti", strTitle.c_str());
    if (nMediaLength > 0) {
        xmlFile.writeAttribute("length", nMediaLength);
    }

    xmlFile.writeEndElement();
    return ERR_OK;
}

MLMsgCmdBatchSearch::MLMsgCmdBatchSearch() {
    bOnlyActiveCanDownload = false;
}

int MLMsgCmdBatchSearch::fromXML(SXNode *pXmlNode) {
    CXMLMsgCmd::fromXML(pXmlNode);

    bOnlyActiveCanDownload = tobool(pXmlNode->getPropertyInt("oncd", false));

    listSearchItems.reserve(pXmlNode->listChildren.size());

    for (SXNode::LIST_CHILDREN::iterator it = pXmlNode->listChildren.begin();
    it != pXmlNode->listChildren.end(); ++it)
        {
        SXNode *p = *it;
        if (strcmp(p->name.c_str(), MZ_ITEM) != 0) {
            continue;
        }
        listSearchItems.push_back(MLSearchItem());
        listSearchItems.back().fromXML(p);
    }
    if (listSearchItems.size() == 0) {
        return ERR_CMD_PARAM;
    }

    return ERR_OK;
}

int MLMsgCmdBatchSearch::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_BATCH_SEARCH);

    if (bOnlyActiveCanDownload) {
        xmlFile.writeAttribute("oncd", bOnlyActiveCanDownload);
    }

    CXMLMsgCmd::toXMLAttribute(xmlFile);

    for (MLListSearchItems::iterator it = listSearchItems.begin(); it != listSearchItems.end(); ++it) {
        (*it).toXML(xmlFile);
    }

    xmlFile.writeEndElement();
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLLyricsInfoLite::fromXML(SXNode *pXmlNode) {
    strSaveName = strFile = pXmlNode->getPropertySafe("fn");
    pXmlNode->getPropertyBinData("content", bufLyrContent);

    return ERR_OK;
}

int MLLyricsInfoLite::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_ITEM);

    if (strFile.size()) {
        string bufFile;

        if (readFile(strFile.c_str(), bufFile) && bufFile.size()) {
            xmlFile.writeAttribute("fn", fileGetName(strSaveName.c_str()));
            xmlFile.writeAttribute("content", bufFile.c_str(), bufFile.size());
        } else {
            ERR_LOG1("Failed to open lyrics file: %s", strFile.c_str());
        }
    }

    xmlFile.writeEndElement();
    return ERR_OK;
}

int MLMsgRetBatchSearch::fromXML(SXNode *pXmlNode) {
    int nRet = MLRetMsg::fromXML(pXmlNode);
    if (nRet != ERR_OK) {
        return nRet;
    }

    for (SXNode::LIST_CHILDREN::iterator it = pXmlNode->listChildren.begin();
    it != pXmlNode->listChildren.end(); ++it)
        {
        SXNode *p = *it;
        if (strcmp(p->name.c_str(), MZ_ITEM) != 0) {
            continue;
        }
        listLyricsInfo.push_back(MLLyricsInfoLite());
        listLyricsInfo.back().fromXML(p);
    }

    return ERR_OK;
}

int MLMsgRetBatchSearch::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_RETURN);

    toXMLAttribute(xmlFile);

    for (ListLyricsInfoLite::iterator it = listLyricsInfo.begin();
    it != listLyricsInfo.end(); ++it)
        {
        MLLyricsInfoLite &fi = *it;
        fi.toXML(xmlFile);
    }

    xmlFile.writeEndElement();

    // append data block
    string &buf = xmlFile.getBuffer();
    buf.append(SZ_DATA_BLOCK_SIGN, LEN_DATA_BLOCK_SIGN);

    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdUploadV0::fromXML(SXNode *pXmlNode) {
    strLoginName = pXmlNode->getPropertySafe("username");

    strFileContent = pXmlNode->getPropertySafe("FileDataB64");

    return ERR_OK;
}

int MLMsgCmdUploadV0::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_UPLOADV0);

    xmlFile.writeAttribute("username", strLoginName.c_str());

    xmlFile.writeAttribute("FileDataB64", strFileContent.c_str());
    xmlFile.writeEndElement();
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
int MLMsgCmdUpload::fromXML(SXNode *pXmlNode) {
    strLoginName = pXmlNode->getPropertySafe("username");

    strPwdMask = pXmlNode->getPropertySafe("pwd");

    cstr_t szContent = pXmlNode->getPropertySafe("FileData");
    if (!szContent) {
        return ERR_BAD_FILE_CONTENT;
    }

    {
        int nContentLen = (int)strlen(szContent);
        // Convert FileData from base64
        if (nContentLen < 6) {
            return ERR_BAD_FILE_CONTENT;
        }

        // Convert Base64 data >> UTF8
        if (!base64Decode(szContent, nContentLen, strFileContent)) {
            return ERR_BAD_BASE64CONTENT;
        }
    }

    return ERR_OK;
}

int MLMsgCmdUpload::toXML(CXMLWriter &xmlFile) {
    xmlFile.writeStartElement(MZ_UPLOAD);

    xmlFile.writeAttribute("username", strLoginName.c_str());
    xmlFile.writeAttribute("pwd", strPwdMask.c_str());

    // Convert UTF8 >> Base64 str
    string encodedContent = base64Encode((uint8_t *)strFileContent.c_str(), strFileContent.size());

    xmlFile.writeAttribute("FileData", encodedContent.c_str());
    xmlFile.writeEndElement();
    return ERR_OK;
}

int MLMsgRetUpload::fromXML(SXNode *pXmlNode) {
    int nRet = MLRetMsg::fromXML(pXmlNode);
    if (nRet != ERR_OK) {
        return nRet;
    }

    strLyricsId = pXmlNode->getPropertySafe("LyricsId");

    return ERR_OK;
}

void MLMsgRetUpload::toXMLAttribute(CXMLWriter &xmlFile) {
    MLRetMsg::toXMLAttribute(xmlFile);
    if (strLyricsId.size()) {
        xmlFile.writeAttribute("LyricsId", strLyricsId.c_str());
    }
}


//////////////////////////////////////////////////////////////////////////
//

CMLPacketAccountMgr *CMLPacketAccountMgr::m_pMLPacketWrapper = nullptr;

CMLPacketAccountMgr::CMLPacketAccountMgr() {
    m_packFlag = MPV_V1_MD5_ID;
    m_bEnableXmlPack = true;
    m_nInvalidV1PackCount = 0;
    m_nInvalidPackCount = 0;
    m_bServer = true;
}

const char* CMLPacketAccountMgr::GetPwd(long nAccountId) {
    MAP_ACCOUNT::iterator it;

    it = m_mapAccount.find(nAccountId);
    if (it == m_mapAccount.end()) {
        return nullptr;
    } else {
        return (*it).second.c_str();
    }
}

CMLPacketAccountMgr *CMLPacketAccountMgr::getInstance() {
    if (!m_pMLPacketWrapper) {
        m_pMLPacketWrapper = new CMLPacketAccountMgr;
    }

    return m_pMLPacketWrapper;
}

void CMLPacketAccountMgr::addAccount(long nId, const char*szPwd) {
    m_mapAccount[nId] = szPwd;
}

void CMLPacketAccountMgr::eraseAccount(long nId) {
    m_mapAccount.erase(nId);
}

CMLPacketWrapper::CMLPacketWrapper() {
    m_pAccountMgr = CMLPacketAccountMgr::getInstance();
    m_nCurrentId = 0;
    m_packFlag = m_pAccountMgr->m_packFlag;
}

int CMLPacketWrapper::wrapp(string &buff) {
    if (!m_pAccountMgr->m_bServer) {
        m_packFlag = m_pAccountMgr->m_packFlag;
    }

    if (m_packFlag == MPV_XML) {
        return ERR_OK;
    }

    if (m_packFlag == MPV_V1_MD5_ID) {
        const char *szPwd = m_pAccountMgr->GetPwd(m_nCurrentId);
        if (!szPwd) {
            ERR_LOG1("Invalid Lyrics search Account: %d", m_nCurrentId);
            return ERR_BAD_MSG;
        }

        MLEncodePacketV1 head;
        head.byVersion = MPV_V1_MD5_ID;
        head.setID(m_nCurrentId);

        size_t nSizeOld = buff.size();
        buff += szPwd;
        assert(sizeof(head.byMd5) >= 16);
        md5ToBinary(buff.c_str(), buff.size(), head.byMd5);
        buff.resize(nSizeOld);

        int n = 0;
        uint32_t i;
        for (i = 0; i < buff.size(); i++) {
            n += (uint8_t)(buff[i]);
        }
        head.byXor = uint8_t(n / buff.size());

        for (i = 0; i < buff.size(); i++) {
            buff[i] = buff[i] ^ head.byXor;
        }

        buff.insert(0, (const char *)&head, sizeof(head));
        return ERR_OK;
    }

    ERR_LOG1("CMLPacketWrapper::wrapp() Not supported Packet version: %d", m_packFlag);

    return ERR_OK;
}

int CMLPacketWrapper::unwrapp(string &buff) {
    if (buff.size() <= sizeof(MLEncodePacketV1)) {
        return ERR_INVALID_PACK_VER;
    }

    MLEncodePacketV1 *pHead = (MLEncodePacketV1 *)buff.data();
    if (pHead->byVersion == MPV_XML) {
        // We doesn't support old XML Packet now.
        return ERR_DISABLE_PACK_XML;
    }

    if (pHead->byVersion == MPV_V1_MD5_ID) {
        int i;

        for (i = sizeof(MLEncodePacketV1); i < (int)buff.size(); i++) {
            buff[(uint32_t)i] = buff[(uint32_t)i] ^ pHead->byXor;
        }

        m_nCurrentId = pHead->getID();
        const char *szPwd = m_pAccountMgr->GetPwd(m_nCurrentId);
        if (!szPwd) {
            ERR_LOG1("Invalid Lyrics search Account: %d", m_nCurrentId);
            m_pAccountMgr->m_nInvalidV1PackCount++;
            return ERR_BAD_MSG;
        }

        unsigned char Md5[16];
        int nSizeOld = (int)buff.size();
        buff.append(szPwd);
        md5ToBinary(buff.c_str() + sizeof(MLEncodePacketV1), buff.size() - sizeof(MLEncodePacketV1), Md5);
        buff.resize(nSizeOld);

        // buff is appended and resized, pHead might point to incorrect memory, assign again.
        pHead = (MLEncodePacketV1 *)buff.data();
        for (i = 0; i < CountOf(pHead->byMd5); i++) {
            if (pHead->byMd5[i] != Md5[i]) {
                ERR_LOG2("Invalid MD5 Value, Account Error: %d, %s", m_nCurrentId, szPwd);
                m_pAccountMgr->m_nInvalidV1PackCount++;
                return ERR_PACK_BAD_MD5;
            }
        }
        buff.erase(0, sizeof(MLEncodePacketV1));

        if (m_pAccountMgr->m_bServer) {
            m_packFlag = MPV_V1_MD5_ID;
        }

        return ERR_OK;
    }

    ERR_LOG1("CMLPacketWrapper::unwrapp() Invalid Packet version: %d", pHead->byVersion);
    m_pAccountMgr->m_nInvalidPackCount++;

    return ERR_INVALID_PACK_VER;
}

//////////////////////////////////////////////////////////////////////////
//

#define ALLOC_BLOCK_LEN        (2 * 1024)

void saveRequestToFile(string *buf) {
    static int i = 0;
    char fn[256];
    snprintf(fn, sizeof(fn), "d:\\request_%d.bin", i++);

    writeFile(fn, buf->data(), buf->size());
}

int CMLProtocol::readMsg(MLMsgType &msgType, string *pMsgBuff) {
    assert(m_pNetFile);
    assert(pMsgBuff->size() == 0);
    pMsgBuff->clear();

    int nRet = m_pNetFile->readExactly(*pMsgBuff, 4);
    if (nRet != ERR_OK) {
        return nRet;
    }
    if (pMsgBuff->size() < 4) {
        return ERR_BAD_MSG;
    }

    char *pszMsg;
    pszMsg = (char *)pMsgBuff->data();

    if (iStartsWith(pszMsg, "HTTP")) {
        msgType = MT_HTTP_RET;
    } else if (iStartsWith(pszMsg, "GET")) {
        msgType = MT_HTTP_GET;
    } else if (iStartsWith(pszMsg, "POST")) {
        msgType = MT_HTTP_POST;
    } else {
        return readOldXmlMsg(msgType, pMsgBuff);
    }

    //
    // HTTP protocol message
    //
    CHttpProtocol *httpProtocol = nullptr;

    if (msgType == MT_HTTP_GET || msgType == MT_HTTP_POST) {
        httpProtocol = &m_httpRequest;
    } else if (msgType == MT_HTTP_RET) {
        httpProtocol = &m_httpReturn;
    } else {
        assert(0);
    }

    httpProtocol->setCacheData(pMsgBuff->data(), (int)pMsgBuff->size());
    httpProtocol->setNetFile(m_pNetFile);
    nRet = httpProtocol->readHead();
    if (nRet != ERR_OK) {
        return nRet;
    }

    pMsgBuff->clear();
    nRet = httpProtocol->readData(pMsgBuff, ML_MSG_MAX_LENGTH);
    if (nRet != ERR_OK) {
        return nRet;
    }

    // Is XML or Binary XML?
    if ((msgType == MT_HTTP_POST && strcmp(m_httpRequest.getUrl(), SZ_ML_POST_URL) == 0)
        || msgType == MT_HTTP_RET) {
        // saveRequestToFile(pMsgBuff);

        nRet = m_packetWrapper.unwrapp(*pMsgBuff);
        if (nRet != ERR_OK) {
            return nRet;
        }

        if (pMsgBuff->c_str()[0] == '<') {
            msgType = MT_HTTP_XML;
        } else if (pMsgBuff->c_str()[0] == 'M') {
            msgType = MT_HTTP_BIN_XML;
        } else {
            return ERR_BAD_MSG;
        }
    }

    return ERR_OK;
}

int CMLProtocol::readOldXmlMsg(MLMsgType &msgType, string *pMsgBuff) {
    if (pMsgBuff->size() < ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN) {
        int nRet = m_pNetFile->readExactly(*pMsgBuff, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN - (int)pMsgBuff->size());
        if (nRet != ERR_OK) {
            return nRet;
        }
    }

    // read message header
    cstr_t pszMsg = nullptr;
    pszMsg = pMsgBuff->data();
    if (!startsWith(pszMsg, ML_MSG_XML)) {
        return ERR_BAD_MSG;
    }

    char szMsgDataLen[ML_MSG_XML_DATA_LEN + 1];
    strncpy_safe(szMsgDataLen, CountOf(szMsgDataLen), pszMsg + ML_MSG_XML_LEN, ML_MSG_XML_DATA_LEN);
    szMsgDataLen[ML_MSG_XML_DATA_LEN] = '\0';

    char *pTemp = szMsgDataLen;
    while (*pTemp == ' ') {
        pTemp++;
    }
    int nDataLength = atoi(pTemp);
    if (nDataLength > 0xFFFFFF || nDataLength <= 0) {
        pMsgBuff->clear();
        DBG_LOG1("INVALID MSG!! length: %d Invalid! CMGCPCom::OnRecieve(), Drop it!", nDataLength);
        return ERR_BAD_MSG;
    }

    // Erase header
    pMsgBuff->erase(0, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN);

    int nRet = m_pNetFile->readExactly(*pMsgBuff, nDataLength - (int)pMsgBuff->size());
    if (nRet != ERR_OK) {
        return nRet;
    }

    if (pMsgBuff->size() >= ML_MSG_MAX_LENGTH) {
        return ERR_OUTOF_BUFF;
    }

    msgType = MT_XML;

    nRet = m_packetWrapper.unwrapp(*pMsgBuff);
    if (nRet != ERR_OK) {
        return nRet;
    }

    return ERR_OK;
}

int CMLProtocol::writeOldXmlMsg(string *pMsgBuff) {
    char szHeader[256];
    int nWrite;
    int nRet;

    snprintf(szHeader, CountOf(szHeader), ML_MSG_XML_FMT, (int)pMsgBuff->size());

    nRet = m_pNetFile->write(szHeader, ML_MSG_XML_LEN + ML_MSG_XML_DATA_LEN, &nWrite);
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = m_pNetFile->write(pMsgBuff->data(), (int)pMsgBuff->size(), &nWrite);
    if (nRet != ERR_OK) {
        return nRet;
    }

    return ERR_OK;
}


CMLProtocol::CMLProtocol() {
    m_bUseProxy = false;
    m_pNetFile = nullptr;
}

CMLProtocol::~CMLProtocol() {

}

int CMLProtocol::init(CNetFile *pFile, cstr_t szHost, int nPort) {
    setNetFile(pFile);

    m_httpReqWriteXmlCache.setNetFile(m_pNetFile);
    m_httpReqWriteXmlCache.setHost(szHost, nPort);
    m_httpReqWriteXmlCache.setRequestType(CHttpRequestProtocol::RQ_POST);
    m_httpReqWriteXmlCache.setUrl(SZ_ML_POST_URL);
    m_httpReqWriteXmlCache.setAvailProp(CHttpRequestProtocol::RP_POST_MIN);
    //     m_httpReqWriteXmlCache.setPropValue(CHttpRequestProtocol::RP_HOST, "www.minilyrics.com");
    m_httpReqWriteXmlCache.setPropValue(CHttpRequestProtocol::RP_USER_AGENT, "MiniLyrics");

    m_httpRetWriteXmlCache.setNetFile(m_pNetFile);
    m_httpRetWriteXmlCache.setAvailProp(CHttpReturnProtocol::RP_MIN_RET);
    m_httpRetWriteXmlCache.setServer("MiniLyrics Server/1.1");
    m_httpRetWriteXmlCache.setConnection("close");

    return ERR_OK;
}

void CMLProtocol::setProxy(bool bEnableProxy, cstr_t szProxyServer, int nProxyPort, cstr_t szBase64ProxyUserPass) {
    m_httpReqWriteXmlCache.setProxyBase64Pass(bEnableProxy, szBase64ProxyUserPass);
}

// ERR_OK
int CMLProtocol::writeXMLMsg(MLMsgType msgType, string *pMsgBuff) {
    int nRet;

    nRet = m_packetWrapper.wrapp(*pMsgBuff);
    if (nRet != ERR_OK) {
        return nRet;
    }

    if (msgType == MT_HTTP_POST) {
#ifdef _DEBUG
        if (g_profile.getInt("PrintMLSend", false)) {
            DBG_LOGDUMPS(pMsgBuff->data(), pMsgBuff->size());
        }
#endif
        m_httpReqWriteXmlCache.setNetFile(m_pNetFile);
        m_httpReqWriteXmlCache.setContentLength(pMsgBuff->size());
        m_httpReqWriteXmlCache.writeHead();
        return m_httpReqWriteXmlCache.writeData(pMsgBuff->data(), pMsgBuff->size());
    } else if (msgType == MT_HTTP_RET) {
        m_httpRetWriteXmlCache.setNetFile(m_pNetFile);
        m_httpRetWriteXmlCache.setContentLength(pMsgBuff->size());
        m_httpRetWriteXmlCache.writeHead();
        return m_httpRetWriteXmlCache.writeData(pMsgBuff->data(), pMsgBuff->size());
    } else {
        return writeOldXmlMsg(pMsgBuff);
    }
}

int CMLProtocol::parseXMLMsg(string *pMsgBuff, CSimpleXML &xml) {
    if (!xml.parseData(pMsgBuff->data(), pMsgBuff->size())) {
        return ERR_PARSE_XML;
    }

    return ERR_OK;
}


MLMsgCmd CMLProtocol::getMsgCommand(CSimpleXML &xml) {
    SXNode *pNode;
    pNode = xml.m_pRoot;
    if (pNode) {
        return (MLMsgCmd)stringToID(__MlCmdIDStr, pNode->name.c_str(), MC_UNKNOWN);
    }
    return MC_UNKNOWN;
}


#if UNIT_TEST

#include "../TinyJS/utils/unittest.h"


TEST(MLProtocol, ParseBinXml) {
    MLMsgRetBatchSearch msgRet;

    msgRet.result = ERR_CMD_NOT_MATCH;
    msgRet.strMessage = "Msg";

    string strFile = getUnittestTempDir();
    strFile += "unittest_save.txt";
    string bufContent = ";alskge\1,.zsm,ng\n;ea<>>\n";
    ASSERT_TRUE(writeFile(strFile.c_str(), bufContent.c_str(), bufContent.size()));

    {
        MLLyricsInfoLite lyrInfo;
        lyrInfo.strFile = strFile;
        lyrInfo.strSaveName = "artist - title.txt";
        msgRet.listLyricsInfo.push_back(lyrInfo);
    }
    MLLyricsInfoLite lyrInfo;
    lyrInfo.strFile = strFile;
    lyrInfo.strSaveName = "artist - title2.txt";
    msgRet.listLyricsInfo.push_back(lyrInfo);

    CMLBinXMLWriter xmlStream;
    int ret = msgRet.toXML(xmlStream);
    ASSERT_TRUE(ret == ERR_OK);

    deleteFile(strFile.c_str());

    CSimpleXML xml;
    ret = xml.parseData(xmlStream.getBuffer().c_str(), xmlStream.getBuffer().size());
    ASSERT_TRUE(ret != false);
    MLMsgRetBatchSearch msgFrom;
    ret = msgFrom.fromXML(xml.m_pRoot);
    ASSERT_TRUE(ret == ERR_OK);

    ASSERT_TRUE(msgFrom.result == msgRet.result);
    ASSERT_TRUE(msgFrom.strMessage == msgRet.strMessage);
    ASSERT_TRUE(msgFrom.listLyricsInfo.size() == msgRet.listLyricsInfo.size());
    ListLyricsInfoLite::iterator itFrom = msgFrom.listLyricsInfo.begin();
    for (ListLyricsInfoLite::iterator it = msgRet.listLyricsInfo.begin();
    it != msgRet.listLyricsInfo.end(); ++it, ++itFrom)
        {
        MLLyricsInfoLite &lyr = *it;
        MLLyricsInfoLite &lyrFrom = *itFrom;

        ASSERT_TRUE(lyr.strSaveName == lyrFrom.strSaveName);
        ASSERT_TRUE(lyr.strSaveName == lyrFrom.strFile);
        ASSERT_TRUE(bufContent.size() == lyrFrom.bufLyrContent.size());
        ASSERT_TRUE(memcmp(bufContent.c_str(), lyrFrom.bufLyrContent.c_str(), lyrFrom.bufLyrContent.size()) == 0);
    }
}

TEST(MLProtocol, MLMsgCmdSearch) {
    MLMsgCmdSearch msgSearch;

    msgSearch.nProductId = PRODUCT_ID_MINILYRICS;
    msgSearch.nRequestPage = 1;
    msgSearch.strArtist = "ar";
    msgSearch.strClient = "client xxx";
    msgSearch.strTitle = "title";

    CMLBinXMLWriter xmlStream;
    int ret = msgSearch.toXML(xmlStream);
    ASSERT_TRUE(ret == ERR_OK);

    CSimpleXML xml;
    ret = xml.parseData(xmlStream.getBuffer().c_str(), xmlStream.getBuffer().size());
    ASSERT_TRUE(ret != false);
    MLMsgCmdSearch msgFrom;
    ret = msgFrom.fromXML(xml.m_pRoot);
    ASSERT_TRUE(ret == ERR_OK);

    ASSERT_TRUE(msgFrom.nRequestPage == msgSearch.nRequestPage);
    ASSERT_TRUE(msgFrom.strArtist == msgSearch.strArtist);
    ASSERT_TRUE(msgFrom.strClient == msgSearch.strClient);
    ASSERT_TRUE(msgFrom.strTitle == msgSearch.strTitle);
}

TEST(MLProtocol, MLMsgCmdUploadResult) {
    MLMsgRetUpload retUpload;

    retUpload.strLyricsId = "lyrics_id";
    retUpload.strMessage = "msg";
    retUpload.strOrgCmd = "org";

    CMLBinXMLWriter xmlStream;
    int ret = retUpload.toXML(xmlStream);
    ASSERT_TRUE(ret == ERR_OK);

    CSimpleXML xml;
    ret = xml.parseData(xmlStream.getBuffer().c_str(), xmlStream.getBuffer().size());
    ASSERT_TRUE(ret != false);
    MLMsgRetUpload retUpload2;
    ret = retUpload2.fromXML(xml.m_pRoot);
    ASSERT_TRUE(ret == ERR_OK);

    ASSERT_TRUE(retUpload2.strLyricsId == retUpload.strLyricsId);
    ASSERT_TRUE(retUpload2.strMessage == retUpload.strMessage);
    ASSERT_TRUE(retUpload2.strOrgCmd == retUpload.strOrgCmd);
}

#endif
