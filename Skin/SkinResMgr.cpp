/********************************************************************
    Created  :    2002/04/03    19:33
    FileName :    ResManager.cpp
    Author   :    xhy

    Purpose  :    管理资源文件，如位图等的打开和关闭等
*********************************************************************/

#include "SkinTypes.h"
#include "Skin.h"
#include "SkinResMgr.h"
#include "../Utils/Utils.h"


CSkinResMgr::CSkinResMgr() {
    m_hue = 0.0;
    m_saturation = 0.0;
    m_luminance = 0.0;
    m_bAdjustHue = false;
}

CSkinResMgr::~CSkinResMgr() {
    map<string, ResourceRef>::iterator it;

    // 释放所有未释放的资源
    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        if ((*it).second.nCount > 0) {
            // ERR_LOG1("Bitmap resource :%d is NOT freed before exit.", (int)(*it).second.image);
            freeRawImage((*it).second.image);
        }
    }

    m_mapBitmap.clear();
}

void CSkinResMgr::onClose() {
    m_bAdjustHue = false;
    m_hue = 0;
    m_saturation = 0;
    m_luminance = 0;
}

RawImageData *CSkinResMgr::loadBitmap(cstr_t resName) {
    if (isEmptyString(resName)) {
        return nullptr;
    }

    //
    // 资源的搜索顺序：szDir1, szDir2, 最后为 resName
    string file;
    if (!getResourcePathName(resName, file)) {
        ERR_LOG1("Bitmap File: %s does NOT exist.\nPlease check the skin xml file.", resName);
        return nullptr;
    }

    auto it = m_mapBitmap.find(file.c_str());
    if (it == m_mapBitmap.end()) {
        // 不存在，则重新加载新的位图
        ResourceRef res;
        res.image = loadRawImageDataFromFile(file.c_str()); // LoadBitmapFromFile(szFile);

        if (res.image == nullptr) {
            ERR_LOG1("Failed to load Image: %s, Please check the skin xml file.", file.c_str());
            return nullptr;
        }

        res.nCount = 1;
        m_mapBitmap[file.c_str()] = res;

        if (m_hue != 0.0) {
            adjustImageHue(res.image, m_hue);
        }

        return res.image;
    } else {
        // 存在，则使用前面的位图句柄
        (*it).second.nCount++;
        return (*it).second.image;
    }
}

void CSkinResMgr::freeBitmap(RawImageData *image) {
    assert(image);

    map<string, ResourceRef>::iterator it;

    // 释放所有未释放的资源
    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        ResourceRef &res = (*it).second;
        if (res.image == image) {
            if (res.nCount <= 1) {
                freeRawImage(res.image);
                m_mapBitmap.erase(it);
            } else {
                res.nCount--;
            }
            return;
        }
    }

    ERR_LOG1("delete, But Not Found Bitmap Object:%d", (int)(long)image);
    // freeRawImage(image);
}

void CSkinResMgr::incBitmapReference(RawImageData *image) {
    assert(image);

    map<string, ResourceRef>::iterator it;

    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        if ((*it).second.image == image) {
            (*it).second.nCount++;
            return;
        }
    }

    ERR_LOG1("add reference, but Not Found Bitmap Object:%d", (int)(long)image);
}

void CSkinResMgr::addRessourceDir(cstr_t szResDir, int nPos) {
    if (isEmptyString(szResDir)) {
        return;
    }

    string strDir = szResDir;
    dirStringAddSep(strDir);

    for (uint32_t i = 0; i < m_vResSearchDirs.size(); i++) {
        if (strcasecmp(strDir.c_str(), m_vResSearchDirs[i].c_str()) == 0) {
            return;
        }
    }

    if (nPos < 0 || nPos >= (int)m_vResSearchDirs.size()) {
        m_vResSearchDirs.push_back(strDir);
    } else {
        m_vResSearchDirs.insert(m_vResSearchDirs.begin() + nPos, strDir);
    }
}

void CSkinResMgr::enumFiles(cstr_t extFilter, vector<string> &vFiles, bool bEnumFullPath) {
    for (uint32_t i = 0; i < m_vResSearchDirs.size(); i++) {
        enumFilesInDir(m_vResSearchDirs[i].c_str(), extFilter, vFiles, bEnumFullPath);
    }
}

bool CSkinResMgr::getResourcePathName(cstr_t szResName, string &fileNameOut) const {
    fileNameOut.clear();

    for (uint32_t i = 0; i < m_vResSearchDirs.size(); i++) {
        fileNameOut = m_vResSearchDirs[i] + szResName;
        if (isFileExist(fileNameOut.c_str())) {
            return true;
        }
    }

    ERR_LOG1("Resource File: %s does NOT exist!", szResName);
    return false;
}

string CSkinResMgr::getResourcePathName(cstr_t szResName) const {
    string fileName;

    getResourcePathName(szResName, fileName);

    return fileName;
}


void CSkinResMgr::adjustHue(float hue, float saturation, float luminance) {
    if (hue == m_hue) {
        return;
    }

    m_hue = hue;
    m_luminance = luminance;
    m_saturation = saturation;

    // adjust the hue of opened images
    map<string, ResourceRef>::iterator it;

    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        ResourceRef &item = (*it).second;
        cstr_t szFile = (*it).first.c_str();

        RawImageData *image = loadRawImageDataFromFile(szFile); // LoadBitmapFromFile(szFile);
        if (image == nullptr) {
            continue;
        }

        item.image->exchange(image);
        freeRawImage(image);

        if (hue != 0) {
            adjustImageHue(item.image, hue);
        }
    }
}

void CSkinResMgr::getAdjustHueParam(float &hue, float &saturation, float &luminance) {
    hue = m_hue;
    saturation = m_saturation;
    luminance = m_luminance;
}

#ifdef _DEBUG
void CSkinResMgr::dbgOutLoadedImages() {
    map<string, ResourceRef>::iterator it;

    if (m_mapBitmap.empty()) {
        return;
    }

    DBG_LOG1("Loaded %d Bmp", m_mapBitmap.size());

    // 释放所有未释放的资源
    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        DBG_LOG2("    FastBmp: %d : %s", (*it).second.nCount, (*it).first.c_str());
    }
}
#endif
