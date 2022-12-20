#include "MPlayerAppBase.h"
#include "MediaDetectionService.h"

CMediaDetectionService        g_mediaDetectionService;

CMediaDetectionService::CMediaDetectionService(void)
{
}

CMediaDetectionService::~CMediaDetectionService(void)
{
}

void listMedia(cstr_t szDir, vector<string> &vFiles)
{
    FileFind        find;
    string            strDir;
    string            strFile;
    // char            szMsg[256];

    if (!find.openDir(szDir))
        return;

    strDir = szDir;
    dirStringAddSep(strDir);

    while (find.findNext())
    {
        if (find.isCurDir())
        {
            if (strcmp(".", find.getCurName()) != 0 &&
                strcmp("..", find.getCurName()) != 0)
            {
                strFile = strDir + find.getCurName();
                listMedia(strFile.c_str(), vFiles);
            }
        }
        else
        {
            if (g_Player.isExtAudioFile(fileGetExt(find.getCurName())))
            {
                strFile = strDir + find.getCurName();
                vFiles.push_back(strFile);
            }
        }
    }
}

void CMediaDetectionService::addMediaInDir(cstr_t szDir)
{
    vector<string>                vFiles;

    listMedia(szDir, vFiles);

    addMedia(vFiles);
}

void CMediaDetectionService::addMedia(vector<string> &vFiles)
{
    CMPAutoPtr<IMediaLibrary>    pMediaLib;
    int                            nRet;
    //char                        szMsg[256];
    int                            n = 0;


//     sprintf(szMsg, _TLT("%d files added"), 0);
//     setMsg1(szMsg);
//     setMsg2(_TLT("Adding files..."));

    if (g_Player.getMediaLibrary(&pMediaLib) == ERR_OK)
    {
        for (int i = 0; i < (int)vFiles.size(); i++)
        {
            CMPAutoPtr<IMedia>            pMedia;

            if (pMediaLib->getMediaByUrl(vFiles[i].c_str(), &pMedia) == ERR_OK)
                continue;

            nRet = pMediaLib->add(vFiles[i].c_str(), &pMedia);
            if (nRet == ERR_OK)
            {
                n++;
//                 sprintf(szMsg, _TLT("%d files added"), n);
//                 setMsg1(szMsg);
            }
        }
    }

//    setMsg2(_TLT("The library has been updated. To continue, click Done."));
}
