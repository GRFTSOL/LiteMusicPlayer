#include "MPlayerAppBase.h"
#include "SkinListCtrlEx.h"
#include "../Skin/SkinEditCtrl.h"
#include "MPlaylistSearchObj.h"
#include "PlayListFile.h"


#define SZ_RECENT_PL            "RecentPL"
#define COUNT_RECENT_PL_MAX        10


void splitKeywords(cstr_t szKeywords, VecStrings &vKeywords)
{
    cstr_t        pStart, p;
    string        str;

    vKeywords.clear();

    pStart = p = szKeywords;
    while (*p)
    {
        while (isAlpha(*p) || isDigit(*p) || (unsigned int)(*p) >= 128)
            p++;

        if (p == pStart)
            p++;

        if (*pStart != ' ' && *pStart != '\t')
        {
            vKeywords.push_back(string());
            vKeywords.back().append(pStart, p);
        }

        pStart = p;
    }
}


static uint32_t searchWithKeywordEx(VecStrings &vKeywords, cstr_t szText, CSkinListCtrlEx::ItemStringEx::VecTextColor &vItemClrs)
{
    cstr_t        pStart, p, pLastUnmatch;
    string        str;
    VecStrings        vLeftKeywords = vKeywords;

    pLastUnmatch = pStart = p = szText;
    while (*p)
    {
        while (isAlpha(*p) || isDigit(*p))
            p++;

        if (p == pStart)
            p++;

        if (*pStart != ' ' && *pStart != '\t')
        {
            for (VecStrings::size_type i = 0; i < vLeftKeywords.size(); i++)
            {
                if (int(p - pStart) == vLeftKeywords[i].size()
                    && strncasecmp(pStart, vLeftKeywords[i].c_str(), vLeftKeywords[i].size()) == 0)
                {
                    vLeftKeywords.erase(vLeftKeywords.begin() + i);

                    // set unmatched color
                    if (pLastUnmatch != pStart)
                        vItemClrs.add(int(pStart - pLastUnmatch), CSkinListCtrlEx::CN_TEXT);

                    // set matched color
                    vItemClrs.add(int(p - pStart), CSkinListCtrlEx::CN_CUSTOMIZED_START);

                    pLastUnmatch = p;
                    break;
                }
            }
            if (vLeftKeywords.empty())
                return 1;
        }

        pStart = p;
    }

    if (vLeftKeywords.empty())
        return 1;

    return 0;
}


static uint32_t searchWithKeyword(cstr_t szKeyword, cstr_t szText, CSkinListCtrlEx::ItemStringEx::VecTextColor &vItemClrs)
{
    cstr_t        pKey, pDst, pDstStart;

    vItemClrs.clear();

    pDst = szText;

    while (*pDst)
    {
        pDstStart = pDst;
        pKey = szKeyword;

        while (*pDst && toLower(*pKey) == toLower(*pDst))
        {
            pKey++;
            pDst++;
        }

        if (*pKey == 0)
        {
            if (int(pDstStart - szText) > 0)
                vItemClrs.add(int(pDstStart - szText), CSkinListCtrlEx::CN_TEXT);
            vItemClrs.add(int(pDst - pDstStart), CSkinListCtrlEx::CN_CUSTOMIZED_START);
            return uint32_t(-1);
        }

        if (*pDst)
            pDst++;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPlaylistSearchObj, "PlaylistSearchObj")

CMPlaylistSearchObj::CMPlaylistSearchObj()
{
    m_pCtrlEdit = nullptr;
    m_pCtrlPlaylist = nullptr;
    m_pCtrlPlaylistList = nullptr;
    m_nIDBeginSearchTimer = 0;
}


CMPlaylistSearchObj::~CMPlaylistSearchObj()
{
    m_pSkin->unregisterUIObjNotifyHandler(this);
}


void CMPlaylistSearchObj::onCreate()
{
    CSkinDataObj::onCreate();

    m_pCtrlEdit = (CSkinEditCtrl *)m_pSkin->getUIObjectById(
        m_pSkin->getSkinFactory()->getIDByName(m_strIDEditor.c_str()),
        CSkinEditCtrl::className());
    if (m_pCtrlEdit)
        m_pCtrlEdit->setEditNotification(this);

    m_pCtrlPlaylist = (CSkinListCtrlEx *)m_pSkin->getUIObjectById(
        m_pSkin->getSkinFactory()->getIDByName(m_strIDPlaylist.c_str()),
        CSkinListCtrlEx::className());

    m_pCtrlPlaylistList = (CSkinListCtrl *)m_pSkin->getUIObjectById(
        m_pSkin->getSkinFactory()->getIDByName(m_strIDPlaylistList.c_str()),
        CSkinListCtrl::className());

//    CColor        clrKeyword(RGB(0, 128, 255));
//    m_pCtrlPlaylist->setColor(CSkinListCtrl::CN_CUSTOMIZED_START, clrKeyword);

    if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
        return;

    m_pSkin->registerUIObjNotifyHandler(m_pCtrlPlaylistList->getID(), this);
    m_pSkin->registerUIObjNotifyHandler(m_pCtrlPlaylist->getID(), this);

    m_pCtrlPlaylistList->addColumn("Playlist", 200);
    m_pCtrlPlaylist->addColumn("Results", 200);

    m_pCtrlPlaylistList->insertItem(PN_NOWPLAYING, _TLT("Now Playing"));
    m_vRecentPL.push_back("NowPlaying");

    m_pCtrlPlaylistList->insertItem(PN_ALL_MEDIALIB, _TLT("Entire Media Library"));
    m_vRecentPL.push_back("Entire Media Library");

    // load recent playlist...
    for (int i = PN_CUSTOMIZED; i < COUNT_RECENT_PL_MAX; i++)
    {
        string            strPlaylist;

        strPlaylist = g_profile.getString(stringPrintf("%s%d", SZ_RECENT_PL, i).c_str(), "");
        if (strPlaylist.empty())
            break;
        if (!isFileExist(strPlaylist.c_str()))
            continue;

        addInRecentPlaylist(fileGetTitle(strPlaylist.c_str()).c_str(), strPlaylist.c_str(), -1);
    }
    m_pCtrlPlaylistList->invalidate();
    m_pCtrlPlaylistList->setItemSelectionState(0, true);
}


void CMPlaylistSearchObj::onTimer(int nId)
{
    string            strKeyword;

    if (m_nIDBeginSearchTimer)
    {
        m_pSkin->unregisterTimerObject(this, m_nIDBeginSearchTimer);
        m_nIDBeginSearchTimer =  0;
    }

    if (!m_pCtrlEdit || !m_pCtrlPlaylist)
        return;

    strKeyword = m_pCtrlEdit->getText();

    trimStr(strKeyword);
    strKeyword = toLower(strKeyword.c_str());
    doSearch(strKeyword.c_str());
}


void CMPlaylistSearchObj::doSearch(cstr_t szKeyword)
{
    MLRESULT    nRet;

    // clear old results
    if (m_PlaylistResults)
        m_PlaylistResults.release();
    m_pCtrlPlaylist->deleteAllItems(false);

    assert(m_Playlist);
    if (!m_Playlist)
        g_Player.getCurrentPlaylist(&m_Playlist);

    if (isEmptyString(szKeyword))
    {
        m_PlaylistResults = m_Playlist;
        updatePlaylist();

        return;
    }

    if (g_Player.newPlaylist(&m_PlaylistResults) != ERR_OK)
        return;

    //
    // search playlist with keyword: szKeyword
    //

    CSkinListCtrlEx::ItemStringEx::VecTextColor        vItemClrs;
    uint32_t            nMatchValue;
    VecStrings            vKeywords;

    splitKeywords(szKeyword, vKeywords);

    for (uint32_t i = 0; i < m_Playlist->getCount(); i++)
    {
        CMPAutoPtr<IMedia>    media;
        nRet = m_Playlist->getItem(i, &media);
        if (nRet == ERR_OK)
        {
            string str = CPlayer::formatMediaTitle(media);

            nMatchValue = searchWithKeyword(szKeyword, str.c_str(), vItemClrs);
            if (nMatchValue > 0)
            {
                m_pCtrlPlaylist->insertItem(-1, str.c_str(), vItemClrs, 0, 0, false);
                m_PlaylistResults->insertItem(-1, media);
            }
            else
            {
                nMatchValue = searchWithKeywordEx(vKeywords, str.c_str(), vItemClrs);
                if (nMatchValue > 0)
                {
                    m_pCtrlPlaylist->insertItem(-1, str.c_str(), vItemClrs, 0, 0, false);
                    m_PlaylistResults->insertItem(-1, media);
                }
            }
        }
    }

    m_pCtrlPlaylist->invalidate();
}


bool CMPlaylistSearchObj::setProperty(cstr_t szProperty, cstr_t szValue)
{
    if (CSkinDataObj::setProperty(szProperty, szValue))
        return true;

    if (isPropertyName(szProperty, "KeywordEdit"))
        m_strIDEditor = szValue;
    else if (isPropertyName(szProperty, "Playlist"))
        m_strIDPlaylist = szValue;
    else if (isPropertyName(szProperty, "PlaylistList"))
        m_strIDPlaylistList = szValue;
    else
        return false;

    return true;
}

#ifdef _SKIN_EDITOR_
void CMPlaylistSearchObj::enumProperties(CUIObjProperties &listProperties)
{
    CSkinDataObj::enumProperties(listProperties);

}
#endif // _SKIN_EDITOR_


void CMPlaylistSearchObj::onTextChanged()
{
    assert(m_pCtrlEdit);
    if (!m_pCtrlEdit)
        return;

    if (m_nIDBeginSearchTimer != 0)
        m_pSkin->unregisterTimerObject(this, m_nIDBeginSearchTimer);

    m_nIDBeginSearchTimer = m_pSkin->registerTimerObject(this, TD_BEGIN_SEARCH);
}


void CMPlaylistSearchObj::onSpecialKey(SpecialKey key)
{
    if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
        return;

    if (key == IEditNotification::SK_ENTER)
        useResultAsNowPlaying();
}


void CMPlaylistSearchObj::onUIObjNotify(IUIObjNotify *pNotify)
{
    if (!m_pCtrlEdit || !m_pCtrlPlaylist || !m_pCtrlPlaylistList)
        return;

    if (pNotify->nID == m_pCtrlPlaylistList->getID())
    {
        //
        // process Playlist List command
        //
        if (pNotify->isKindOf(CSkinListCtrl::className()))
        {
            CSkinListCtrlEventNotify    *pSkinListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;

            int nSel = m_pCtrlPlaylistList->getNextSelectedItem();
            if (nSel < 0 || nSel >= (int)m_vRecentPL.size())
                return;

            switch (pSkinListCtrlNotify->cmd)
            {
            case CSkinListCtrlEventNotify::C_SEL_CHANGED:
            case CSkinListCtrlEventNotify::C_DBL_CLICK:
            case CSkinListCtrlEventNotify::C_ENTER:
                {
                    // Choose as current search source.
                    if (m_Playlist)
                        m_Playlist.release();
                    if (nSel == PN_NOWPLAYING)
                        g_Player.getCurrentPlaylist(&m_Playlist);
                    else if (nSel == PN_ALL_MEDIALIB)
                    {
                        CMPAutoPtr<IMediaLibrary>        mediaLib;
                        if (g_Player.getMediaLibrary(&mediaLib) == ERR_OK)
                            mediaLib->getAll(&m_Playlist, MLOB_NONE, -1);
                    }
                    else
                    {
                        if (g_Player.newPlaylist(&m_Playlist) != ERR_OK)
                            return;
                        ::loadPlaylist(g_Player.getIMPlayer(), m_Playlist, m_vRecentPL[nSel].c_str());
                    }
                    m_PlaylistResults = m_Playlist;
                    if (pSkinListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_SEL_CHANGED)
                    {
                        // open selected playlist
                        g_Player.setCurrentPlaylist(m_Playlist);
                        g_Player.play();
                    }
                    updatePlaylist();
                }
                break;
            case CSkinListCtrlEventNotify::C_KEY_DELETE:
                // Erase recent playlist
                if (nSel != 0)
                {
                    m_vRecentPL.erase(m_vRecentPL.begin() + nSel);
                    m_pCtrlPlaylistList->deleteItem(nSel);
                }
                break;
            default:
                break;
            }
        }
    }
    else if (pNotify->nID == m_pCtrlPlaylist->getID())
    {
        //
        // process Playlist command
        //
        if (pNotify->isKindOf(CSkinListCtrl::className()))
        {
            CSkinListCtrlEventNotify    *pSkinListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;

            int nSel = m_pCtrlPlaylist->getNextSelectedItem();
            if (nSel == -1)
                return;

            switch (pSkinListCtrlNotify->cmd)
            {
            case CSkinListCtrlEventNotify::C_DBL_CLICK:
            case CSkinListCtrlEventNotify::C_ENTER:
                {
                    // Use current result as now playing
                    useResultAsNowPlaying();
                }
                break;
            case CSkinListCtrlEventNotify::C_KEY_DELETE:
                {
                    // Erase selected items in result list
                    vector<int>        vSelItems;
                    int                nItem = -1, count;

                    while ((nItem = m_pCtrlPlaylist->getNextSelectedItem(nItem)) != -1)
                        vSelItems.push_back(nItem);

                    // sort selected items descending
                    sort(vSelItems.begin(), vSelItems.end());

                    count = m_PlaylistResults->getCount();
                    if (count == 0)
                        return;

                    // remove selected items descending
                    for (int n = 0; n < (long)vSelItems.size(); n++)
                    {
                        nItem = vSelItems[n];
                        assert(nItem < count && nItem >= 0);
                        if (m_PlaylistResults->removeItem(nItem)!= ERR_OK)
                            break;
                        m_pCtrlPlaylist->deleteItem(nItem);
                    }
                }
                break;
            default:
                break;
            }
        }
    }
}


void CMPlaylistSearchObj::updatePlaylist()
{
    assert(m_pCtrlPlaylist);

    m_pCtrlPlaylist->deleteAllItems(false);

    for (uint32_t i = 0; i < m_PlaylistResults->getCount(); i++)
    {
        CMPAutoPtr<IMedia>    media;
        if (m_PlaylistResults->getItem(i, &media) == ERR_OK)
        {
            string str = CPlayer::formatMediaTitle(media);
            m_pCtrlPlaylist->insertItem(-1, str.c_str(), 0, 0, false);
        }
    }

    m_pCtrlPlaylist->invalidate();
}

void CMPlaylistSearchObj::addInRecentPlaylist(cstr_t szName, cstr_t szFile, int nPos)
{
    VecStrings::size_type i;

    for (i = PN_CUSTOMIZED; i < m_vRecentPL.size(); i++)
    {
        string            str;
        m_pCtrlPlaylistList->getItemText(i, 0, str);
        if (strcasecmp(str.c_str(), szName) == 0)
        {
            m_vRecentPL[i] = szFile;
            m_pCtrlPlaylistList->setItemText(i, 0, szName);
            return;
        }
    }

    if (i == m_vRecentPL.size())
    {
        if (nPos == -1)
        {
            m_vRecentPL.push_back(szFile);
            m_pCtrlPlaylistList->insertItem(nPos, szName);
        }
        else
        {
            m_vRecentPL.insert(m_vRecentPL.begin() + nPos, szFile);
            m_pCtrlPlaylistList->insertItem(nPos, szName);
        }
    }
}

void CMPlaylistSearchObj::useResultAsNowPlaying()
{
    assert(m_pCtrlPlaylist);

    if (!m_pCtrlPlaylist || !m_PlaylistResults || m_PlaylistResults->getCount() == 0)
        return;

    string            strKeyword;
    string            strPlaylistFile;
    int                i;
    int                nSel;

    nSel = m_pCtrlPlaylist->getNextSelectedItem();
    if (nSel == -1)
        nSel = 0;

    g_Player.setCurrentPlaylist(m_PlaylistResults);
    g_Player.setCurrentMediaInPlaylist(nSel);
    g_Player.play();

    strKeyword = m_pCtrlEdit->getText();
    trimStr(strKeyword);

    strPlaylistFile = getAppDataDir();
    strPlaylistFile += fileNameFilterInvalidChars(strKeyword.c_str());
    strPlaylistFile += ".m3u";
    savePlaylistAsM3u(m_PlaylistResults, strPlaylistFile.c_str());

    // insert in PL list or replace it
    addInRecentPlaylist(strKeyword.c_str(), strPlaylistFile.c_str(), PN_CUSTOMIZED);

    // Only show about COUNT_RECENT_PL_MAX results
    if (m_vRecentPL.size() > COUNT_RECENT_PL_MAX)
        m_vRecentPL.resize(COUNT_RECENT_PL_MAX);
    for (i = COUNT_RECENT_PL_MAX; i < m_pCtrlPlaylistList->getItemCount(); i++)
        m_pCtrlPlaylistList->deleteItem(i, true);
    m_pCtrlPlaylistList->invalidate();

    // save recent playlist...
    for (i = PN_CUSTOMIZED; i < (int)m_vRecentPL.size(); i++)
        g_profile.writeString(stringPrintf("%s%d", SZ_RECENT_PL, i).c_str(), m_vRecentPL[i].c_str());
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

IMPLEMENT_CPPUNIT_TEST_REG(MPlaylistSearchObj)

class CTestCasePlaylistSearchObj : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCasePlaylistSearchObj);
    CPPUNIT_TEST(testSplitKeywords);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp()
    {
    }
    void tearDown() 
    {
    }

protected:
    void testSplitKeywords()
    {
        {
            cstr_t    szKeyWords = "";
            cstr_t    vOutput[] = { "" };
            verifySplitResult(szKeyWords, vOutput, 0);
        }

        {
            cstr_t    szKeyWords = "a";
            cstr_t    vOutput[] = { "a" };
            verifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
        }

        {
            cstr_t    szKeyWords = "abcdefg";
            cstr_t    vOutput[] = { "abcdefg" };
            verifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
        }

        {
            cstr_t    szKeyWords = "a, b  cc;d-!ee?ff";
            cstr_t    vOutput[] = { "a", ",", "b", "cc", ";", "d", "-", 
                "!", "ee", "?", "ff" };
            verifySplitResult(szKeyWords, vOutput, CountOf(vOutput));
        }
    }

    void verifySplitResult(cstr_t szKeywords, cstr_t vOutput[], int nCountOutput)
    {
        VecStrings    vKeywords;

        splitKeywords(szKeywords, vKeywords);
        if (nCountOutput == vKeywords.size())
        {
            for (int i = 0; i < nCountOutput; i++)
            {
                if (strcmp(vOutput[i], vKeywords[i].c_str()) != 0)
                    CPPUNIT_FAIL_T(stringPrintf("splitKeywords:%s Failed, %s should be %s, Round: %d", szKeywords, vKeywords[i].c_str(), vOutput[i], i).c_str());
            }
        }
        else
            CPPUNIT_FAIL_T(stringPrintf("splitKeywords:%s Failed, Output Count not match.", szKeywords).c_str());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCasePlaylistSearchObj);


#endif // _CPPUNIT_TEST
