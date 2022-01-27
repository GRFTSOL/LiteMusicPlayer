// ImgPngFile.h: interface for the CImgPngFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMGPNGFILE_H__771FEED8_2574_419C_80EB_987F913F28A3__INCLUDED_)
#define AFX_IMGPNGFILE_H__771FEED8_2574_419C_80EB_987F913F28A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImgFileBase.h"

class CImgPngFile : public CImgFileBase  
{
public:
    CImgPngFile();
    virtual ~CImgPngFile();

    virtual bool open(cstr_t szFile);
    virtual bool save(cstr_t szFile);

};

#endif // !defined(AFX_IMGPNGFILE_H__771FEED8_2574_419C_80EB_987F913F28A3__INCLUDED_)
