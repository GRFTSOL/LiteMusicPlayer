//
//  SkinIcons.hpp
//

#pragma once

#ifndef SkinIcons_h
#define SkinIcons_h

#include "SFImage.h"


class CSkinResMgr;

struct SkinIconFile {
    string                      fileName;
    CSFImage                    image;
    uint32_t                    cx, cy;
    VecStrings                  names;

    std::map<string, uint32_t>  nameToIndex;
};

using ListSkinIconFiles = std::list<SkinIconFile>;


class SkinIcons {
public:
    void clear();
    void load(CSkinResMgr *mgr);

    bool getImage(cstr_t name, CSFImage &image);

protected:
    ListSkinIconFiles loadSkinIcon();

protected:
    ListSkinIconFiles           _skinIconFiles;

};

#endif /* SkinIcons_h */
