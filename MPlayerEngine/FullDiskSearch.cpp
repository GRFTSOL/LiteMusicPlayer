#include "FullDiskSearch.h"
#include "base.h"
#include <process.h>
#include <winioctl.h>

//#include "mutex.h"


///////////////////////////////////////////////////////////////////////////////
// startFullDiskSearch()
// create a low priority task to start search
///////////////////////////////////////////////////////////////////////////////
bool CFullDiskSearch::start(void) {
    if (!m_threadWorking.create(seachWholeDisk, this)) {
        return false;
    }

    m_threadWorking.setPriority(THREAD_PRIORITY_IDLE);

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// cbIsFileIntereasted()
///////////////////////////////////////////////////////////////////////////////
bool CFullDiskSearch::cbIsFileIntereasted(const char* szFile) {
    const char * const aExtensions[] = {"mp3", "wav", "ape"};
    const char* szExt = nullptr;

    szExt = fileGetExt(szFile);

    if('\0' == szExt[0]) {
        return false;
    }


    for (int i = 0; i < CountOf(aExtensions); i++) {
        if (0 == strcasecmp(aExtensions[i], szExt)) {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnNewFile(const char* path, const char* file) {
    // User Debug tracer tool to view the log instantly.
    DBG_LOG2("NewFile: %s%s", path, file);
}

///////////////////////////////////////////////////////////////////////////////
// cbOnNewFile()
// media library will set call back function to add or update media info
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnRemoveFile(const char* path, const char* file) {
}

///////////////////////////////////////////////////////////////////////////////
// cbOnEndOfFolder()
// media library will set call back function to remove all existing items under this folder
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::cbOnEndOfFolder(const char* path) {
    // update media library and notify UI
    DBG_LOG1("End of scan folder: %s", path);
}

///////////////////////////////////////////////////////////////////////////////
// searchFolder()
// actually search a folder
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::searchFolder(const char* szStr, /* "C:\" */ bool bRoot) {
    FileFind find;

    while (!isDiskIdle(nullptr)) {
        sleep(2000);
    }

    if (!find.openDir(szStr)) {
        return;
    }

    string searchStr = szStr;

    while (find.findNext()) {
        if (find.isCurDir()) {
            string subFolder = szStr;
            subFolder += find.getCurName();
            subFolder += PATH_SEP_STR;
            searchFolder(subFolder.c_str(), false);
        } else if (cbIsFileIntereasted(find.getCurName())) {
            // ???? Should sleep on adding every file?
            cbOnNewFile(szStr, find.getCurName());
        }
    }

    cbOnEndOfFolder(szStr);
}

///////////////////////////////////////////////////////////////////////////////
// seachWholeDisk()
// entry of working thread
///////////////////////////////////////////////////////////////////////////////
void CFullDiskSearch::seachWholeDisk(void* param) {
    CFullDiskSearch* p = (CFullDiskSearch*)param;
    // enum all disk with the type

    VecStrings vDrives;

    getLogicalDrives(vDrives);

    for (unsigned int i = 0; i < vDrives.size(); i++) {
        // search this disk
        p->searchFolder(vDrives[i].c_str(), true);
    }
}
