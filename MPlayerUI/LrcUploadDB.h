// LrcUploadDB.h: interface for the CLrcUploadDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_)
#define AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_

#include "../third-parties/sqlite/Sqlite3.hpp"


struct LrcUploadFileInfo
{
    enum UPLOADSTATE
    {
        US_UPLOAD_OK,
        US_EXIST,
        US_UPLOADERROR
    };

    char    szFileName[MAX_PATH];
    uint32_t    dwUploadState;
};

class CLrcUploadDB  
{
public:
    int init();

    bool searchByFileName(cstr_t szFileName, LrcUploadFileInfo &lyrUploadInfo);

    int add(LrcUploadFileInfo &lrcUpload);

    static cstr_t uploadStateToStr(uint32_t dwUploadState);

public:
    CLrcUploadDB();
    virtual ~CLrcUploadDB();

protected:
    CSqlite3        m_db;

};

#endif // !defined(AFX_LRCUPLOADDB_H__FBA46183_DF30_40C3_AE73_8359343BC5B8__INCLUDED_)
