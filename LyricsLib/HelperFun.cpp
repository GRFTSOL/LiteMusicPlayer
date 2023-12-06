#include "HelperFun.h"
#ifdef WIN32
#include "mbstring.h"
#endif

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

#if UNIT_TEST

#include "../TinyJS/utils/unittest.h"

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
