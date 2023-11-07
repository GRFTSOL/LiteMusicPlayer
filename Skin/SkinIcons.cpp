//
//  SkinIcons.cpp
//

#include "SkinIcons.h"
#include "SkinResMgr.h"
#include "SkinTypes.h"


void SkinIcons::clear() {
    _skinIconFiles.clear();
}

void SkinIcons::load(CSkinResMgr *mgr) {
    assert(mgr);
    auto files = mgr->getResourcesWithNames("icons.xml");

    for (auto &fn : files) {
        CSimpleXML xml;
        if (xml.parseFile(fn.c_str())) {
            auto *root = xml.root();

            if (root->name != "icons") {
                ERR_LOG1("Invalid icons.xml format: %s", fn.c_str());
                continue;
            }

            for (auto child : root->listChildren) {
                if (child->name == "file") {
                    SkinIconFile file;
                    auto filename = child->getProperty("filename");
                    auto names = child->getProperty("names");
                    if (filename && names) {
                        file.image.loadFromSRM(mgr, filename, getScreenScaleFactor());
                        file.fileName = filename;
                        file.cx = child->getPropertyInt("cx");
                        file.cy = child->getPropertyInt("cy");
                        file.image.setHeight(file.cy);
                        StringView(names).split(',', file.names);
                        trimStr(file.names);

                        int i = 0;
                        for (auto &name : file.names) {
                            file.nameToIndex[name] = i++;
                        }

                        _skinIconFiles.push_back(file);
                    } else {
                        ERR_LOG1("Invalid icons.xml format, no filename or names property: %s", fn.c_str());
                    }
                }
            }
        }
    }
}

bool SkinIcons::getImage(cstr_t name, CSFImage &image) {
    for (auto &file : _skinIconFiles) {
        auto it = file.nameToIndex.find(name);
        if (it != file.nameToIndex.end()) {
            auto index = (*it).second;
            image = file.image;
            image.setX(file.cx * index);
            image.setWidth(file.cx);
            return true;
        }
    }

    return false;
}
