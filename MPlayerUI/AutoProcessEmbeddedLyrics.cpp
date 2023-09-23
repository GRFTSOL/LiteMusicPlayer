#include "MPlayerApp.h"
#include "AutoProcessEmbeddedLyrics.h"


CAutoProcessEmbeddedLyrics g_autoProcessEmbeddedLyrics;

#define SZ_JOB_FILE_NAME    "process_embedded_lyrics.bxml"
#define VERSION_CUR         "2"
#define SZ_VERSION          "version"

//////////////////////////////////////////////////////////////////////

CAutoProcessEmbeddedLyrics::CAutoProcessEmbeddedLyrics() {

}

CAutoProcessEmbeddedLyrics::~CAutoProcessEmbeddedLyrics() {

}

#ifdef UNICODE
static void strSplit(cstr_t szStr, char chSeperator, vector<string> &vAnsiStr) {
    VecStrings vStr;
    strSplit(szStr, chSeperator, vStr);
    for (uint32_t i = 0; i < vStr.size(); i++) {
        string str;
        stringToAnsi(vStr[i].c_str(), vStr[i].size(), str);
        vAnsiStr.push_back(str);
    }
}
#endif

bool CAutoProcessEmbeddedLyrics::Item::fromXML(SXNode *pNode) {
    m_strSongFile = pNode->getPropertySafe("song_file");
    strSplit(pNode->getPropertySafe("embedded_lyr_names"), '|', m_vLyrNames);
    m_bRemove = tobool(pNode->getPropertyInt("remove", false));
    m_nTryFailed = pNode->getPropertyInt("try_failed", 0);

    string binLyrics;
    if (pNode->getPropertyBinData("bin_lyrics", binLyrics)) {
        m_strLyrics.insert(0, binLyrics.c_str(), binLyrics.size());
    }

    return m_strSongFile.size() > 0;
}

void CAutoProcessEmbeddedLyrics::Item::toXML(CXMLWriter &xmlWriter) {
    xmlWriter.writeStartElement("item");

    xmlWriter.writeAttribute("song_file", m_strSongFile.c_str());
    xmlWriter.writeAttribute("embedded_lyr_names",
        strJoin(m_vLyrNames.begin(), m_vLyrNames.end(), "|").c_str());
    xmlWriter.writeAttribute("remove", m_bRemove);
    xmlWriter.writeAttribute("try_failed", m_nTryFailed);
    xmlWriter.writeAttribute("bin_lyrics", m_strLyrics.c_str(), m_strLyrics.size());

    xmlWriter.writeEndElement();
}

int CAutoProcessEmbeddedLyrics::saveEmbeddedLyrics(cstr_t szSongFile, const string &lyrics, const VecStrings &vEmbeddedLyrNames) {
    if (vEmbeddedLyrNames.empty()) {
        return ERR_OK;
    }

    if (!MediaTags::canSaveEmbeddedLyrics(szSongFile)) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    MutexAutolock autolock(m_mutex);

    for (auto it = m_listJobs.begin(); it != m_listJobs.end(); ++it) {
        Item &item = *it;
        if (strcmp(szSongFile, item.m_strSongFile.c_str()) == 0) {
            item.m_bRemove = false;
            item.m_strLyrics = lyrics;
            item.m_vLyrNames = vEmbeddedLyrNames;
            item.m_nTryFailed = 0;
            int nRet = dealEmbeddedLyrics(item);
            if (nRet == ERR_OK) {
                m_listJobs.erase(it);
            }
            return ERR_OK;
        }
    }

    Item newItem;
    newItem.m_strSongFile = szSongFile;
    newItem.m_bRemove = false;
    newItem.m_strLyrics = lyrics;
    newItem.m_vLyrNames = vEmbeddedLyrNames;
    newItem.m_nTryFailed = 0;
    int nRet = dealEmbeddedLyrics(newItem);
    if (nRet != ERR_OK) {
        m_listJobs.push_back(newItem);
    }

    return nRet;
}

int CAutoProcessEmbeddedLyrics::removeEmbeddedLyrics(cstr_t szSongFile, const VecStrings &vEmbeddedLyrNames, bool bAddToQueueOnFailed) {
    MutexAutolock autolock(m_mutex);

    for (auto it = m_listJobs.begin(); it != m_listJobs.end(); ++it) {
        Item &item = *it;
        if (strcmp(szSongFile, item.m_strSongFile.c_str()) == 0) {
            item.m_bRemove = true;
            item.m_vLyrNames = vEmbeddedLyrNames;
            item.m_nTryFailed = 0;
            int nRet = dealEmbeddedLyrics(item);
            if (nRet == ERR_OK) {
                m_listJobs.erase(it);
            }
            return ERR_OK;
        }
    }

    Item newItem;
    newItem.m_strSongFile = szSongFile;
    newItem.m_bRemove = true;
    newItem.m_vLyrNames = vEmbeddedLyrNames;
    newItem.m_nTryFailed = 0;
    int nRet = dealEmbeddedLyrics(newItem);
    if (nRet != ERR_OK && bAddToQueueOnFailed) {
        m_listJobs.push_back(newItem);
    }

    return nRet;
}

void CAutoProcessEmbeddedLyrics::removeJob(cstr_t szSongFile) {
    MutexAutolock autolock(m_mutex);

    for (auto it = m_listJobs.begin(); it != m_listJobs.end(); ++it) {
        Item &item = *it;
        if (strcmp(szSongFile, item.m_strSongFile.c_str()) == 0) {
            m_listJobs.erase(it);
            return;
        }
    }
}

void CAutoProcessEmbeddedLyrics::init() {
    loadJobs(true);
}

bool CAutoProcessEmbeddedLyrics::onPostQuit() {
    onSongChanged();

    if (m_listJobs.empty()) {
        return true;
    }

    return false;
}

//
// save embedded lyrics of previous song file.
//
void CAutoProcessEmbeddedLyrics::onSongChanged() {
    MutexAutolock autolock(m_mutex);

    string strSongFileCur = g_player.getSrcMedia();

    for (ListItems::iterator it = m_listJobs.begin(); it != m_listJobs.end(); ) {
        Item &item = *it;

        const int MAX_RETRY_COUNT = 3;
        if (item.m_nTryFailed <= MAX_RETRY_COUNT
            && strcmp(strSongFileCur.c_str(), item.m_strSongFile.c_str()) != 0) {
            if (dealEmbeddedLyrics(item) == ERR_OK) {
                // remove succeeded job
                it = m_listJobs.erase(it);
                continue;
            } else {
                item.m_nTryFailed++;
            }
        }
        ++it;
    }
}

int CAutoProcessEmbeddedLyrics::dealEmbeddedLyrics(Item &item) {
    if (!MediaTags::canSaveEmbeddedLyrics(item.m_strSongFile.c_str())) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    setFileNoReadOnly(item.m_strSongFile.c_str());

    int nRet;
    if (item.m_bRemove) {
        nRet = dealRemoveEmbeddedLyrics(item);
    } else {
        nRet = dealSaveEmbeddedLyrics(item);
    }

    if (nRet == ERR_OK) {
        item.m_bDealOK = true;
    }

    return nRet;
}

int CAutoProcessEmbeddedLyrics::dealSaveEmbeddedLyrics(Item &item) {
    int ret = MediaTags::saveEmbeddedLyrics(item.m_strSongFile.c_str(), item.m_vLyrNames, item.m_strLyrics);
    if (ret == ERR_OK) {
        item.m_nSucceededCount++;
    }

    return ret;
}

int CAutoProcessEmbeddedLyrics::dealRemoveEmbeddedLyrics(Item &item) {
    assert(item.m_bRemove);

    return MediaTags::removeEmbeddedLyrics(item.m_strSongFile.c_str(), item.m_vLyrNames, &item.m_nSucceededCount);
}

void CAutoProcessEmbeddedLyrics::saveJobs() {
    CMLBinXMLWriter xmlWriter;
    MutexAutolock autolock(m_mutex);

    xmlWriter.writeStartElement("embedded_lyr_jobs");
    xmlWriter.writeAttribute(SZ_VERSION, VERSION_CUR);

    for (ListItems::iterator it = m_listJobs.begin(); it != m_listJobs.end(); ++it) {
        Item &item = *it;
        if (!item.m_bDealOK) {
            item.toXML(xmlWriter);
        }
    }

    xmlWriter.writeEndElement();

    string strFile = getAppDataDir();
    strFile += SZ_JOB_FILE_NAME;
    xmlWriter.saveAsFile(strFile.c_str());
}

void CAutoProcessEmbeddedLyrics::loadJobs(bool bDeleteFile) {
    string strFile = getAppDataDir();
    strFile += SZ_JOB_FILE_NAME;

    m_listJobs.clear();

    if (!isFileExist(strFile.c_str())) {
        return;
    }

    CSimpleXML xml;
    if (xml.parseFile(strFile.c_str())
        && strcmp(xml.m_pRoot->getProperty(SZ_VERSION), VERSION_CUR) == 0) {
        for (SXNode::LIST_CHILDREN::iterator it = xml.m_pRoot->listChildren.begin();
        it != xml.m_pRoot->listChildren.end(); ++it)
            {
            SXNode *pNode = *it;
            if (strcmp(pNode->name.c_str(), "item") != 0) {
                continue;
            }

            Item item;
            if (item.fromXML(pNode)) {
                m_listJobs.push_back(item);
            }
        }
    }

    if (bDeleteFile) {
        deleteFile(strFile.c_str());
    }
}

#if UNIT_TEST

#include "utils/unittest.h"


TEST(AutoProcessEmbeddedLyrics, AutoProcessEmbeddedLyricsLoad) {
    string strFile = getAppDataDir();
    strFile += SZ_JOB_FILE_NAME;

    // Only run this test when the file doesn't exist.
    if (isFileExist(strFile.c_str())) {
        return;
    }

    CAutoProcessEmbeddedLyrics embedded_lyr_process;
    CAutoProcessEmbeddedLyrics::Item item;

    item.m_strSongFile = "C:\\abc.mp3";
    item.m_strLyrics.append(";lksjd;gioew1230$/;%^&*");
    item.m_vLyrNames.push_back("abc");
    item.m_vLyrNames.push_back("de");
    item.m_bRemove = true;
    item.m_bDealOK = false;
    item.m_nTryFailed = 2;
    embedded_lyr_process.m_listJobs.push_back(item);

    item.m_strSongFile = "C:\\abcd.mp3";
    item.m_strLyrics.append(";lksjd;gssgeeggioew1230$/;%^&*");
    item.m_vLyrNames.push_back("a");
    item.m_vLyrNames.push_back("def");
    item.m_bRemove = false;
    item.m_bDealOK = false;
    item.m_nTryFailed = 0;
    embedded_lyr_process.m_listJobs.push_back(item);

    CAutoProcessEmbeddedLyrics::ListItems listJobs = embedded_lyr_process.m_listJobs;

    embedded_lyr_process.saveJobs();
    embedded_lyr_process.loadJobs(true);

    ASSERT_TRUE(listJobs.size() == embedded_lyr_process.m_listJobs.size());

    CAutoProcessEmbeddedLyrics::ListItems::iterator it1 = embedded_lyr_process.m_listJobs.begin();
    for (CAutoProcessEmbeddedLyrics::ListItems::iterator it2 = listJobs.begin();
    it2 != listJobs.end(); ++it1, ++it2)
        {
        CAutoProcessEmbeddedLyrics::Item &i1 = *it1;
        CAutoProcessEmbeddedLyrics::Item &i2 = *it2;

        ASSERT_TRUE(i1.m_strSongFile == i2.m_strSongFile);
        ASSERT_TRUE(i1.m_vLyrNames == i2.m_vLyrNames);
        ASSERT_TRUE(i1.m_bRemove == i2.m_bRemove);
        ASSERT_TRUE(i1.m_nTryFailed == i2.m_nTryFailed);

        ASSERT_TRUE(i1.m_strLyrics.size() == i2.m_strLyrics.size());
        ASSERT_TRUE(memcmp(i1.m_strLyrics.c_str(), i2.m_strLyrics.c_str(), i1.m_strLyrics.size()) == 0);
    }
}

#endif
