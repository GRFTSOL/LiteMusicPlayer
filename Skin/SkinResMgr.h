#pragma once

/********************************************************************
    Created  :    2002/04/03    19:34
    FileName :    ResManager.h
    Author   :    xhy

    Purpose  :    管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#ifndef Skin_SkinResMgr_h
#define Skin_SkinResMgr_h

class RawImageData;


class CSkinResMgr {
public:
    struct ResourceRef {
        RawImageData                *image;
        int                         nCount;
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

    void freeBitmap(RawImageData *image);
    void incBitmapReference(RawImageData *image);

    RawImageData *loadBitmap(cstr_t szBmp);

    void adjustHue(float hue, float saturation, float luminance);

    void getAdjustHueParam(float &hue, float &saturation, float &luminance);

#ifdef _DEBUG
    void dbgOutLoadedImages();
#endif
protected:

protected:
    map<string, ResourceRef>            m_mapBitmap;

    VecStrings                  m_vResSearchDirs;

    bool                        m_bAdjustHue;

    float                       m_hue;
    float                       m_saturation;
    float                       m_luminance;

};

#endif // !defined(Skin_SkinResMgr_h)
