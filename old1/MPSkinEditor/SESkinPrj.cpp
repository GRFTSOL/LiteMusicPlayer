// SESkinPrj.cpp: implementation of the CSESkinPrj class.
//
//////////////////////////////////////////////////////////////////////

#include "SESkinPrj.h"
#include "MPSkinEditor.h"
#include "../MPShared/MLCmd.h"

#include "MainFrm.h"

CSESkinFactory        g_skinFactory;

CSESkinPrj            g_skinPrj;

CSEUIAdapter        g_seUIAdapter;

//////////////////////////////////////////////////////////////////////////
CSEUIAdapter::CSEUIAdapter()
{

}

CSEUIAdapter::~CSEUIAdapter()
{

}

void CSEUIAdapter::setMousePointerInfo(int x, int y)
{
    CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
    CStatusBar    *pStatusBar = pFrame->getStatusBar();
    int            nIndex = 1;//pStatusBar->CommandToIndex(IDC_MOUSE_POINTER);
    pStatusBar->SetPaneStyle(nIndex, pStatusBar->GetPaneStyle(nIndex) & ~SBPS_DISABLED);
    pStatusBar->SetPaneText(nIndex, CStrPrintf("X: %4d Y: %4d", x, y).c_str(), true);
//    pStatusBar->SetPaneText(2, CStrPrintf("X: %4d Y: %4d", x, y).c_str());
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinPrj::CSESkinPrj()
{
    g_skinFactory.SetStringIDMap(g_StringIdMap);
}

CSESkinPrj::~CSESkinPrj()
{

}

void getBackupFile(cstr_t szDir, string &strLatestBackup, string &strNextBackup)
{
    VecStrings    vFiles;
    int        nMax = 0, nMin = 0;
    int        i;
    cstr_t SZ_SKIN_FILE_PREFIX    = "zikiplayer-";
    int        nPrefixLen = strlen(SZ_SKIN_FILE_PREFIX);
    string    strDirOK = szDir;

    dirStringAddSlash(strDirOK);

    // get Max and Min backup file
    enumFilesInDir(szDir, "*.xml", vFiles, false);
    for (i = 0; i < vFiles.size(); i++)
    {
        cstr_t        szFile = vFiles[i].c_str();
        if (strncasecmp(szFile, SZ_SKIN_FILE_PREFIX, nPrefixLen) == 0)
        {
            int        n = atoi(szFile + nPrefixLen);
            if (n > nMax)
                nMax = n;
            else if (n < nMin && n > 0)
                nMin = nMin;
        }
    }

    strLatestBackup = strDirOK + CStrPrintf("%s%03d.xml", SZ_SKIN_FILE_PREFIX, nMax).c_str();
    if (isFileExist(strLatestBackup.c_str()))
        nMax++;
    strNextBackup = strDirOK + CStrPrintf("%s%03d.xml", SZ_SKIN_FILE_PREFIX, nMax).c_str();

    // remove too old backup file
    for (i = nMin; i <= nMax - 1000; i++)
    {
        deleteFile(CStrPrintf("%s%03d.xml", SZ_SKIN_FILE_PREFIX, i).c_str());
    }
}

bool isFileContentSame(cstr_t szFile1, cstr_t szFile2)
{
    string        buff1, buff2;
    if (!readFile(szFile1, buff1))
        return false;
    if (!readFile(szFile2, buff2))
        return false;

    if (buff1.size() != buff2.size())
        return false;

    if (memcmp(buff1.c_str(), buff2.c_str(), buff1.size()) == 0)
        return true;

    return false;
}

bool CSESkinPrj::isModified()
{
    CSkinFileXML    *pSkinFile;
    string            strSkinFile, strSkinFileTemp;

    pSkinFile = g_skinFactory.getSkinFile();
    strSkinFileTemp = getAppResourceDir();
    strSkinFileTemp += "savetempskin.xml";
    strSkinFile = pSkinFile->getFileName();

    // update skin data to xml data.
    for (int i = 0; i < m_vSkinDataItems.size(); i++)
    {
        m_vSkinDataItems[i]->onSave();
    }

    // convert dynamic command node to xml data
    CDynamicCmds &dc = g_skinFactory.getDynamicCmds();
    pSkinFile->setDynamicCmdsNode(dc.toXML());

    // convert dynamic control node to xml data
    CDynamicCtrls &dctrl = g_skinFactory.getDynamicCtrls();
    pSkinFile->SetDynamicCtrlsNode(dupSXNode(dctrl.getData()));

    if (pSkinFile->save(strSkinFile.c_str()) != ERR_OK)
        return true;

    // Is current file same as content?
    return !isFileContentSame(strSkinFileTemp.c_str(), strSkinFile.c_str());
}

int CSESkinPrj::save()
{
    CSkinFileXML    *pSkinFile;
    char            szSkinBackupDir[MAX_PATH];
    string            strSkinFile;

    pSkinFile = g_skinFactory.getSkinFile();
    strSkinFile = pSkinFile->getFileName();

    assert(!strSkinFile.empty());

    //
    // Backup old zikiplayer.xml first.
    //
    string            strLatestBackup, strNextBackup;

    fileGetPath(szSkinBackupDir, CountOf(szSkinBackupDir), strSkinFile.c_str());
    _tcscat(szSkinBackupDir, "backup\\");

    getBackupFile(szSkinBackupDir, strLatestBackup, strNextBackup);
    if (!isDirExist(szSkinBackupDir))
        createDirectory(szSkinBackupDir, nullptr);

    // Is current file same as latest backup?
    if (!isFileContentSame(strLatestBackup.c_str(), strSkinFile.c_str()))
    {
        // backup current
        copyFile(strSkinFile.c_str(), strNextBackup.c_str(), false);
    }

    // update skin data to xml data.
    for (int i = 0; i < m_vSkinDataItems.size(); i++)
    {
        m_vSkinDataItems[i]->onSave();
    }

    // convert dynamic command node to xml data
    CDynamicCmds &dc = g_skinFactory.getDynamicCmds();
    pSkinFile->setDynamicCmdsNode(dc.toXML());

    // convert dynamic control node to xml data
    CDynamicCtrls &dctrl = g_skinFactory.getDynamicCtrls();
    pSkinFile->SetDynamicCtrlsNode(dupSXNode(dctrl.getData()));

    return pSkinFile->save(strSkinFile.c_str());
}

void CSESkinPrj::close()
{
    assert(m_vSkinDataItems.size() == 0);
    m_vSkinDataItems.clear();
    g_skinFactory.close();
}

void CSESkinPrj::onCreateSkinDataItem(ISESkinDataItem *pSkinDataItem)
{
    for (int i = 0; i < m_vSkinDataItems.size(); i++)
    {
        if (m_vSkinDataItems[i] == pSkinDataItem)
            return;
    }

    m_vSkinDataItems.push_back(pSkinDataItem);
}

void CSESkinPrj::onDestroySkinDataItem(ISESkinDataItem *pSkinDataItem)
{
    for (int i = 0; i < m_vSkinDataItems.size(); i++)
    {
        if (m_vSkinDataItems[i] == pSkinDataItem)
        {
            pSkinDataItem->onSave();

            m_vSkinDataItems.erase(m_vSkinDataItems.begin() + i);
            return;
        }
    }
    assert(0);
}
