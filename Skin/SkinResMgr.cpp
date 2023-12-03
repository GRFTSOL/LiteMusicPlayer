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
#include "../third-parties/Agg/include/agg_basics.h"


CSkinResMgr::CSkinResMgr() {
    m_hue = 0.0;
    m_saturation = 0.0;
    m_luminance = 0.0;
    m_bAdjustHue = false;
}

CSkinResMgr::~CSkinResMgr() {
    m_mapBitmap.clear();
}

void CSkinResMgr::onClose() {
    m_bAdjustHue = false;
    m_hue = 0;
    m_saturation = 0;
    m_luminance = 0;
}

RawImageDataPtr CSkinResMgr::loadBitmap(cstr_t resName, float scaleFactor) {
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

    int fileScale = 1;
    for (int n = ceil(scaleFactor); n > 1; n--) {
        string str = file;
        fileSetExt(str, "");
        str += stringPrintf("@%dx", n) + fileGetExt(file.c_str());
        if (isFileExist(str.c_str())) {
            file = str;
            fileScale = n;
            break;
        }
    }

    RawImageDataPtr image;
    string key = file + stringPrintf("-%f", scaleFactor);
    auto it = m_mapBitmap.find(key);
    if (it != m_mapBitmap.end()) {
        image = (*it).second.image.lock();
    }

    if (image == nullptr) {
        // 不存在，则重新加载新的位图
        image = loadImageFile(file.c_str(), scaleFactor, fileScale);
        if (image == nullptr) {
            return nullptr;
        }

        ResourceRef res;
        res.file = file;
        res.image = image;
        res.scaleFactor = scaleFactor;
        res.fileScaleFactor = fileScale;
        m_mapBitmap[key.c_str()] = res;
    }

    return image;
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

VecStrings CSkinResMgr::getResourcesWithNames(cstr_t name) {
    VecStrings files;

    for (auto &path : m_vResSearchDirs) {
        auto file = path + name;
        if (isFileExist(file.c_str())) {
            files.push_back(file);
        }
    }

    return files;
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

    for (auto &item : m_mapBitmap) {
        ResourceRef &res = item.second;

        RawImageDataPtr orgImage = res.image.lock();
        if (orgImage) {
            // 调整了 hue，重新加载所有的图片.
            auto image = loadImageFile(res.file.c_str(), res.scaleFactor, res.fileScaleFactor);
            assert(image);
            if (image) {
                orgImage->exchange(image.get());
            }
        }
    }
}

void CSkinResMgr::getAdjustHueParam(float &hue, float &saturation, float &luminance) {
    hue = m_hue;
    saturation = m_saturation;
    luminance = m_luminance;
}

#ifdef DEBUG
void CSkinResMgr::dbgOutLoadedImages() {
    map<string, ResourceRef>::iterator it;

    if (m_mapBitmap.empty()) {
        return;
    }

    DBG_LOG1("Loaded %d Bmp", m_mapBitmap.size());

    // 释放所有未释放的资源
    for (it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it++) {
        DBG_LOG2("    FastBmp: %d : %s", (*it).second.image.use_count(), (*it).first.c_str());
    }
}
#endif

bool stretchBltRawImage(RawImageData *pImageSrc, RawImageData *pImageDst, const CRect &rcDst, const agg::rect_f &rcSrc, BlendPixMode bpm, int nOpacitySrc);

RawImageDataPtr CSkinResMgr::loadImageFile(cstr_t file, float expectedScaleFactor, int fileScaleFactor) {
    RawImageDataPtr image = loadRawImageDataFromFile(file);
    if (image == nullptr) {
        ERR_LOG1("Failed to load Image: %s, not supported image format?", file);
        return nullptr;
    }

    if (m_hue != 0.0) {
        adjustImageHue(image.get(), m_hue);
    }

    if (expectedScaleFactor != fileScaleFactor) {
        // 需要按照 expectedScaleFactor 来缩放 image;
        RawImageDataPtr expectedImage = make_shared<RawImageData>();

        int width = image->width * expectedScaleFactor / fileScaleFactor;
        int height = image->height * expectedScaleFactor / fileScaleFactor;
        expectedImage->create(width, height, image->bitCount);

        agg::rect_f rcSrc(0, 0, image->width, image->height);
        stretchBltRawImage(image.get(), expectedImage.get(), CRect(0, 0, width, height), rcSrc, BPM_COPY, 255);

        return expectedImage;
    }

    return image;
}
