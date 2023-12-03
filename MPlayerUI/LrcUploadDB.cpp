#include "MPlayerApp.h"
#include "LrcUploadDB.h"


IdToString    _id2sUploadState[] = {
    { LrcUploadFileInfo::US_UPLOAD_OK, "Uploaded successfully" },
    { LrcUploadFileInfo::US_EXIST, "Already exists" },
    { LrcUploadFileInfo::US_UPLOADERROR, "Uploaded failed" },
    { 0, nullptr }
};

#define SQL_CREATE_LYRUPLOAD        "CREATE TABLE IF NOT EXISTS LyrUploadInfo"\
    "("\
    "filename text,"\
    "state integer"\
    ");"\
    "CREATE INDEX IF NOT EXISTS SR_filename on LyrUploadInfo (filename);"

#define SQL_DROP_LYRUPLOAD        "DROP TABLE LyrUploadInfo;"\
    "DROP INDEX SR_filename;"

#define SQL_ADD_LYRUPLOAD    "insert into LyrUploadInfo (filename, state) VALUES ("\
                        "?, ?)"

#define SQL_QUERY_LYRUPLOAD "select * from LyrUploadInfo where filename=?"
//#define SQL_QUERY_SEARCHRESULT    "select * from SearchReuslts"

#define SQL_DEL_LYRUPLOAD   "delete from LyrUploadInfo where filename=?"




CLrcUploadDB::CLrcUploadDB() {

}

CLrcUploadDB::~CLrcUploadDB() {
}

int CLrcUploadDB::init() {
    string strFile = getAppDataDir();
    strFile += "dbLyrUpload.db";

    if (getFileLength(strFile.c_str()) >= 1024 * 100) {
        deleteFile(strFile.c_str());
    }

    int nRet = m_db.open(strFile.c_str());
    if (nRet != ERR_OK) {
        return nRet;
    }

    nRet = m_db.exec(SQL_CREATE_LYRUPLOAD);
    if (nRet != SQLITE_OK) {
        return nRet;
    }

    return ERR_OK;
}

bool CLrcUploadDB::searchByFileName(cstr_t szFileName, LrcUploadFileInfo &lyrUploadInfo) {
    int nRet;
    CSqlite3Stmt sqlQuery;

    nRet = sqlQuery.prepare(&m_db, SQL_QUERY_LYRUPLOAD);
    if (nRet != SQLITE_DONE) {
        return false;
    }

    sqlQuery.bindStaticText(1, szFileName, strlen(szFileName));

    nRet = sqlQuery.step();
    if (nRet != ERR_SL_OK_ROW) {
        return false;
    }

    lyrUploadInfo.dwUploadState = sqlQuery.columnInt(1);

    return true;
}

int CLrcUploadDB::add(LrcUploadFileInfo &lrcUpload) {
    int nRet;
    int n = 1;
    CSqlite3Stmt sqlAdd, sqlDelete;

    // delete old result
    nRet = sqlDelete.prepare(&m_db, SQL_DEL_LYRUPLOAD);
    if (nRet != ERR_OK) {
        return nRet;
    }

    sqlDelete.bindStaticText(1, lrcUpload.szFileName, strlen(lrcUpload.szFileName));
    nRet = sqlDelete.step();
    if (nRet != SQLITE_DONE) {
        return nRet;
    }

    sqlDelete.finalize();

    // add result
    nRet = sqlAdd.prepare(&m_db, SQL_ADD_LYRUPLOAD);
    if (nRet != ERR_OK) {
        return nRet;
    }

    n = 1;
    sqlAdd.bindStaticText(n++, lrcUpload.szFileName, strlen(lrcUpload.szFileName));
    sqlAdd.bindInt(n++, lrcUpload.dwUploadState);

    nRet = sqlAdd.step();
    if (nRet != ERR_OK) {
        return nRet;
    }

    return ERR_OK;
}

cstr_t CLrcUploadDB::uploadStateToStr(uint32_t dwUploadState) {
    return idToString(_id2sUploadState, dwUploadState, "Unknown");
}
