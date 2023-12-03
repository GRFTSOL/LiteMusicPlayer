//
//  StatusLog.hpp
//

#ifndef StatusLog_hpp
#define StatusLog_hpp

#include "../../Utils/UtilsTypes.h"
#include "../../TinyJS/utils/FileApi.h"


class StatusLog {
public:
    void init(cstr_t fileName, size_t rotateSize = 1024 * 1024 * 128, int count = 5);

    void startLine();
    void endLine();

    void writeString(const char *name, const string &value) {
        _writeKey(name);
        _writeValue(value.c_str());
    }

    void writeString(const char *name, cstr_t value) {
        _writeKey(name);
        _writeValue(value);
    }

    void writeBool(const char *name, bool value) {
        _writeKey(name);
        _writeValue(value);
    }

    void writeInt(const char *name, int64_t value) {
        _writeKey(name);
        _writeValue(value);
    }

protected:
    void _openFile();
    void _doLogRotate();

    void _writeKey(const char *name);
    void _writeValue(const char *value);
    void _writeValue(int64_t value);

protected:
    bool                        _writeKeys = true;
    string                      _lineValues, _lineKeys;

    string                      _fn, _path, _title, _ext;
    size_t                      _rotateSize = 0, _rotateCount = 5;
    FilePtr                     _fp;

};

#endif /* StatusLog_hpp */
