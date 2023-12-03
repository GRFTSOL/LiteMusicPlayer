#pragma once

#ifndef ImageLib_ImgPngFile_h
#define ImageLib_ImgPngFile_h


#include "ImgFileBase.h"


class CImgPngFile : public CImgFileBase {
public:
    CImgPngFile();
    virtual ~CImgPngFile();

    virtual bool open(cstr_t szFile);
    virtual bool save(cstr_t szFile);

};

#endif // !defined(ImageLib_ImgPngFile_h)
