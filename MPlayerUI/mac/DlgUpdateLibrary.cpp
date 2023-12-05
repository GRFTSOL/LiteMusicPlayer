#include "../../Skin/Skin.h"
#include "DlgUpdateLibrary.h"
#include "MPlayerApp.h"
#include "Player.h"


#define WM_WORKTHREAD_END_NOTIFY (WM_USER+1)

CUpdateLibraryObj::CUpdateLibraryObj() {
    m_pWnd = nullptr;
}

CUpdateLibraryObj::~CUpdateLibraryObj() {
    m_thread.join();
}

void CUpdateLibraryObj::createWorkThread(Window *hWndNotify) {
    m_thread.create(workThread, this);
}

void CUpdateLibraryObj::workThread(void *lpParam) {
    CUpdateLibraryObj *pThis;

    pThis = (CUpdateLibraryObj *)lpParam;
    pThis->doUpdating();
}

void CUpdateLibraryObj::setMsg1(cstr_t szMsg) {
}

void CUpdateLibraryObj::setMsg2(cstr_t szMsg) {
}

//////////////////////////////////////////////////////////////////////////

void CAddMediaObj::doUpdating() {
    char szMsg[256];
    int n = 0;

    if (!m_bAddFiles) {
        sprintf(szMsg, _TLT("%d files found"), 0);
        setMsg1(szMsg);
        setMsg2(_TLT("Searching for files..."));
        listMedia(m_strDir.c_str());
    }

    sprintf(szMsg, _TLT("%d files added"), 0);
    setMsg1(szMsg);
    setMsg2(_TLT("Adding files..."));

    auto mediaLib = g_player.getMediaLibrary();
    for (int i = 0; i < (int)m_vFiles.size(); i++) {
        auto media = mediaLib->getMediaByUrl(m_vFiles[i].c_str());
        if (!media) {
            media = mediaLib->add(m_vFiles[i].c_str());
            if (media) {
                n++;
                sprintf(szMsg, _TLT("%d files added"), n);
                setMsg1(szMsg);
            }
        }
    }

    setMsg2(_TLT("The library has been updated. To continue, click Done."));
}

void CAddMediaObj::listMedia(cstr_t szDir) {
    FileFind find;
    string strDir;
    string strFile;
    char szMsg[256];

    if (!find.openDir(szDir)) {
        return;
    }

    strDir = szDir;
    dirStringAddSep(strDir);

    while (find.findNext()) {
        if (find.isCurDir()) {
            if (strcmp(".", find.getCurName()) != 0 &&
                strcmp("..", find.getCurName()) != 0) {
                strFile = strDir + find.getCurName();
                listMedia(strFile.c_str());
            }
        } else {
            if (g_player.isExtAudioFile(fileGetExt(find.getCurName()))) {
                strFile = strDir + find.getCurName();
                m_vFiles.push_back(strFile);
                sprintf(szMsg, _TLT("%d files found"), m_vFiles.size());
                setMsg1(szMsg);
            }
        }
    }
}




CDlgUpdateLibrary::CDlgUpdateLibrary() {
    m_pWorkObj = nullptr;
}

CDlgUpdateLibrary::~CDlgUpdateLibrary() {
}


bool CDlgUpdateLibrary::doModal(Window *pWndParent, CUpdateLibraryObj *pWorkObj) {
    m_pWorkObj = pWorkObj;

    return true;
}
