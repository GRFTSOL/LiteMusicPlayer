#pragma once

/********************************************************************
    Created  :    2002/04/03    19:34
    FileName :    ResManager.h
    Author   :    xhy

    Purpose  :    管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#ifndef Skin_SkinResMgr_h
#define Skin_SkinResMgr_h

#include "../GfxRaw/RawImage.h"


class RawImageData;
using RawImageDataWeakPtr = std::weak_ptr<RawImageData>;

class CSkinResMgr {
public:
    struct ResourceRef {
        /**
         * 采用 Weak pointer 来管理图片资源的引用.
         */
        RawImageDataWeakPtr         image;
        string                      file;
        float                       scaleFactor;
        int                         fileScaleFactor;
    };

    CSkinResMgr();
    virtual ~CSkinResMgr();

public:
    void onClose();

    void enumFiles(cstr_t extFilter, vector<string> &vFiles, bool bEnumFullPath);

    bool getResourcePathName(cstr_t szResName, string &fileNameOut) const;
    string getResourcePathName(cstr_t szResName) const;

    void clearRessourceDir() { m_vResSearchDirs.clear(); }

    // nPos = -1, append at tail
    void addRessourceDir(cstr_t szResDir, int nPos = -1);

    RawImageDataPtr loadBitmap(cstr_t szBmp, float scaleFactor);

    void adjustHue(float hue, float saturation, float luminance);

    void getAdjustHueParam(float &hue, float &saturation, float &luminance);

    VecStrings getResourcesWithNames(cstr_t name);

#ifdef DEBUG
    void dbgOutLoadedImages();
#endif
protected:
    RawImageDataPtr loadImageFile(cstr_t file, float expectedScaleFactor, int fileScaleFactor);

protected:
    map<string, ResourceRef>    m_mapBitmap;

    VecStrings                  m_vResSearchDirs;

    bool                        m_bAdjustHue;

    float                       m_hue;
    float                       m_saturation;
    float                       m_luminance;

};

#endif // !defined(Skin_SkinResMgr_h)
