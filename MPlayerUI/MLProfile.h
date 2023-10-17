#pragma once

#ifndef MPlayerUI_MLProfile_h
#define MPlayerUI_MLProfile_h


#include "../MLProtocol/MLProtocol.h"


#define HTTP_PROXY_NONE     1
#define HTTP_PROXY_OURS     4

enum DOWN_SAVE_DIR {
    DOWN_SAVE_IN_SONG_DIR       = 0, // save lyrics in the song file folder
    DOWN_SAVE_IN_CUSTOM_DIR     = 1, // save lyrics in specified directory
    DOWN_SAVE_NO_FILE           = 10, // Do not save lyrics as file
    // DOWN_SAVE_TO_MP3        = 3            // 保存为和歌曲相同的名字
};

enum DOWN_SAVE_NAME {
    DOWN_SAVE_NAME_KEEP         = 0, // 保存在歌曲所在目录
    DOWN_SAVE_NAME_AS_SONG_NAME = 1, // 保存为和歌曲相同的名字
};

#define SZ_SECT_SEARCH_FOLDER   "SearverFodler"


class CMLProfile {
public:
    CMLProfile();
    virtual ~CMLProfile();

public:
    static bool inetIsUseProxy();

    static bool inetGetProxy(string &serverOut, int &nPort);

    static cstr_t inetGetBase64ProxyUserPass();

    // The two API provide running drive independent path remembering.
    static bool writeDir(cstr_t szSectName, cstr_t szKeyName, cstr_t szDir);
    static string getDir(cstr_t szSectName, cstr_t szKeyName, cstr_t szDefDir);

};


#endif // !defined(MPlayerUI_MLProfile_h)
