#include "HelperFun.h"
#ifdef WIN32
#include "mbstring.h"
#endif
#include "LrcTag.h"

void getArtistTitleFromFileName(string &strArtist, string &strTitle, cstr_t szFile) {
    string fileTitle = fileGetTitle(szFile);

    if (!strSplit(fileTitle.c_str(), " - ", strArtist, strTitle)
        && !strSplit(fileTitle.c_str(), '-', strArtist, strTitle)) {
        strArtist.resize(0);
        strTitle = fileTitle;
    }

    trimStr(strArtist);
    trimStr(strTitle);
}

string formatMediaTitle(cstr_t szArtist, cstr_t szTitle) {
    string mediaTitle = szArtist;
    if (mediaTitle.size() > 0 && !isEmptyString(szTitle)) {
        mediaTitle += " - ";
    }
    mediaTitle += szTitle;

    return mediaTitle;
}

void analyseProxySetting(cstr_t szProxySetting, char szServer[], int nMaxSize, int &nPort) {
    cstr_t szColon;

    szColon = strchr(szProxySetting, ':');
    if (szColon == nullptr) {
        strcpy_safe(szServer, nMaxSize, szProxySetting);
        nPort = 80;
    } else {
        strncpy_safe(szServer, nMaxSize, szProxySetting,
            int(szColon - szProxySetting));
        nPort = atoi(szColon + 1);
    }
}

#ifdef WIN32
bool getMLEncriptyData(string &strData) {
    char szFile[MAX_PATH];
    string str;
    char szMD5[64];

    GetModuleFileName(getAppInstance(), szFile, CountOf(szFile));

    if (!readFile(szFile, str)) {
        return false;
    }

    md5ToString(str.c_str() + 0x100, str.size() - 0x100, szMD5);

    // 0x51 ~ 0x78
    // decodebase64(str.c_str() + 0x51, 0x78 - 0x51, strData);
    strData.append(str.c_str() + 0x51, 0x78 - 0x51);

    for (uint32_t i = 0; i < 32; i++) {
        strData[i] = strData[i] ^ szMD5[i];
    }

    // 解密之后的数据了，这里存储的是核心数据

    return true;
}
#endif

#if UNIT_TEST

#include "utils/unittest.h"

TEST(HelperFun, testGetArtistTitleFromFileName) {
    string artist, title;
    cstr_t szFile;

    szFile = "c:\\abc\ar - ti-a.mp3";
    getArtistTitleFromFileName(artist, title, szFile);
    ASSERT_TRUE(artist == "ar");
    ASSERT_TRUE(title == "ti-a");

    szFile = "c:\\abc\ar-ti-a.mp3";
    getArtistTitleFromFileName(artist, title, szFile);
    ASSERT_TRUE(artist == "ar");
    ASSERT_TRUE(title == "ti-a");

    szFile = "c:\\abc\ti.mp3";
    getArtistTitleFromFileName(artist, title, szFile);
    ASSERT_TRUE(artist == "");
    ASSERT_TRUE(title == "ti");
}

#endif
