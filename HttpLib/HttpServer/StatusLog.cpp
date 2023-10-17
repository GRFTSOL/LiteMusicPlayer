//
//  StatusLog.cpp
//  LyricsServer
//
//  Created by henry_xiao on 2023/10/16.
//

#include "StatusLog.hpp"
#include "../../TinyJS/utils/os.h"


void StatusLog::init(cstr_t fileName, size_t rotateSize, int count) {
    _fn = fileName;
    _path = fileGetPath(fileName);
    _title = fileGetTitle(fileName) + "-";
    _ext = fileGetExt(fileName);

    _rotateSize = rotateSize;
    _rotateCount = count;

    _openFile();
}

void StatusLog::startLine() {
    _lineKeys.clear();
    _lineValues.clear();

    assert(_fp.isOK());
    if (!_fp.isOK()) {
        return;
    }

    if (_fp.position() >= _rotateSize) {
        _fp.close();
        _doLogRotate();
        _openFile();
    }
}

void StatusLog::endLine() {
    if (!_fp.isOK()) {
        return;
    }

    if (_writeKeys) {
        _fp.write("#" + _lineKeys + "\n");
        _lineKeys.clear();
        _writeKeys = false;
    }

    _fp.write(_lineValues + "\n");
    _lineValues.clear();
}

void StatusLog::_writeKey(const char *name) {
    if (_writeKeys) {
        if (!_lineKeys.empty()) {
            _lineKeys.push_back('\t');
        }
        _lineKeys.append(name);
    }
}

void StatusLog::_writeValue(const char *value) {
    if (!_lineValues.empty()) {
        _lineValues.push_back('\t');
    }
    _lineValues.append(value);
}

void StatusLog::_writeValue(int64_t value) {
    if (!_lineValues.empty()) {
        _lineValues.push_back('\t');
    }
    _lineValues.append(std::to_string(value));
}

void StatusLog::_openFile() {
    auto size = getFileLength(_fn.c_str());
    if (size > 0 && size >= _rotateSize) {
        _doLogRotate();
    }

    _fp.open(_fn, "a+b");
}

void StatusLog::_doLogRotate() {
    if (_path.empty() || _title.empty()) {
        return;
    }

    FileFind finder;

    if (!finder.openDir(_path.c_str())) {
        return;
    }

    StringView title(_title), ext(_ext);
    VecStrings names;

    while (finder.findNext()) {
        StringView name(finder.getCurName());
        if (name.startsWith(title) && name.endsWith(_ext)) {
            names.push_back(finder.getCurName());
        }
    }

    std::sort(names.begin(), names.end(), std::greater<string>());

    while (names.size() >= _rotateCount) {
        auto fn = dirStringJoin(_path, names.back());
        // LOG(INFO) << "Log rotate, remove old file: " << fn;
        deleteFile(fn.c_str());
        names.pop_back();
    }

    auto latestName = names.front();
    int n = atoi(latestName.c_str() + title.len);
    auto dstName = dirStringJoin(_path, _title + std::to_string(n + 1) + _ext);

    int ret = rename(_fn.c_str(), dstName.c_str());
    if (ret != 0) {
        // failed?
    }
}
