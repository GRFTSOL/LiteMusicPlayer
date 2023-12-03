#include "App.h"
#include "../TinyJS/utils/FileApi.h"
#include "../TinyJS/utils/StringEx.h"


class CProfile    g_profile;
class CLog        g_log;

static string g_resourceDir;
static string g_dataDir;

void setAppResourceDir(cstr_t path) {
    g_resourceDir = path;
    dirStringAddSep(g_resourceDir);
}

void setAppDataDir(cstr_t path) {
    g_dataDir = path;
    dirStringAddSep(g_dataDir);
}

const string &getAppResourceDir() {
    return g_resourceDir;
}

string getAppResourceFile(cstr_t name) {
#ifdef _WIN32
    if (StringView(name).strchr('/') != -1) {
        string tmp(name);
        strrep(tmp, '/', PATH_SEP_CHAR);
        return dirStringJoin(g_resourceDir, tmp);
    }
#endif

    return dirStringJoin(g_resourceDir.c_str(), name);
}

const string &getAppDataDir() {
    return g_dataDir;
}

string getAppDataFile(cstr_t name) {
#ifdef _WIN32
    if (StringView(name).strchr('/') != -1) {
        string tmp(name);
        strrep(tmp, '/', PATH_SEP_CHAR);
        return dirStringJoin(g_dataDir, tmp);
    }
#endif

    return dirStringJoin(g_dataDir.c_str(), name);
}
