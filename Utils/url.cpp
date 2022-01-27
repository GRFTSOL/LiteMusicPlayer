#include "UtilsTypes.h"
#include "Error.h"
#include "CharEncoding.h"
#include "StringEx.h"

cstr_t stringFeedUntil(cstr_t begin, cstr_t szUtil, string &buf)
{
	cstr_t p = strstr(begin, szUtil);
	if (!p)
		return NULL;

	buf.clear();
	buf.append(begin, (size_t)(p - begin));
	return p + strlen(szUtil);
}

// szUrl: scheme://domain:port/path?query_string#fragment_id
bool urlParse(cstr_t szUrl, string &scheme, string &domain, int &port, string &path)
{
    cstr_t begin = szUrl;
    cstr_t p;

    // Scheme
    p = stringFeedUntil(begin, "://", scheme);
    if (!p)
        return false;
    begin = p;

    // domain
    p = stringFeedUntil(begin, ":", domain);
    if (p != nullptr)
    {
        begin = p;

        // Port
        port = atoi(begin);

        string strPort;
        p = stringFeedUntil(begin, "/", strPort);
    }
    else
    {
        port = -1;
        p = stringFeedUntil(begin, "/", domain);
    }

    if (p)
        path = p;
    else
        path.clear();

    return true;
}

bool uriIsQuoted(cstr_t str)
{
    return strchr(str, '%') != nullptr;
}

// Convert %20 etc to blank space...
string uriUnquote(cstr_t str)
{
    string out;

    string temp;
    cstr_t p = str;
    while (*p)
    {
        if (*p == '%' && isHexChar(p[1]) && isHexChar(p[2]))
        {
            int a = hexToInt(p[1]);
            int b = hexToInt(p[2]);
            temp.insert(temp.end(), (char)(uint8_t)(a * 16 + b));
            p += 2;
        }
        else
        {
            if (temp.size() > 0)
            {
                out.append(temp);
                temp.clear();
            }
            out.insert(out.end(), *p);
        }

        p++;
    }

    if (temp.size() > 0)
    {
        out.append(temp);
        temp.clear();
    }
    out.insert(out.end(), *p);

    return out;
}

// COMMENT:
//        将汉字、' '、'\t'等转换为 %20....等
void uriQuote(const char *szLocal, string &strInet)
{
    size_t    i = 0;
    char    szTemp[256];

    size_t nLen = strlen(szLocal);
    strInet = "";

    while (i < nLen)
    {
        if (uint8_t(szLocal[i]) > 127)
        {
            snprintf(szTemp, CountOf(szTemp), "%%%2X", (uint8_t)szLocal[i]);
            strInet += szTemp;
            i++;
            if (i < nLen)
            {
                snprintf(szTemp, CountOf(szTemp), "%%%2X", (uint8_t)szLocal[i]);
                strInet += szTemp;
                i++;
            }
        }
        else if (szLocal[i] == '%' && (!isHexChar(szLocal[i + 1]) || !isHexChar(szLocal[i + 2])))
        {
            snprintf(szTemp, CountOf(szTemp), "%%%2X", (uint8_t)szLocal[i]);
            strInet += szTemp;
            i++;
        }
        else if (szLocal[i] == ' ' || szLocal[i] == '\"' ||
            szLocal[i] == '#' ||
            szLocal[i] == '<' || szLocal[i] == '>')
        {
            snprintf(szTemp, CountOf(szTemp), "%%%2X", (uint8_t)szLocal[i]);
            strInet += szTemp;
            i++;
        }
        else
        {
            strInet += szLocal[i];
            i++;
        }
    }
}

void uriQuote(const char *szLocal, char *szInet, int nLenMax)
{
    string        strTemp;

    uriQuote(szLocal, strTemp);

    strcpy_safe(szInet, nLenMax, strTemp.c_str());
}

bool isUnicodeStr(const void *lpData)
{
    uint8_t                        *szBuffer;

    szBuffer = (uint8_t *)lpData;

    return (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE);
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(StringEx)

    void testStringFeedUntil()
    {
        cstr_t p, begin, szUtil;
        string buf;

        begin = "http://domain";
        szUtil = ":";
        p = stringFeedUntil(begin, szUtil, buf);
        CPPUNIT_ASSERT(p == begin + 5);
        CPPUNIT_ASSERT(buf == "http");

        p = stringFeedUntil(begin, "*", buf);
        CPPUNIT_ASSERT(p == nullptr);

        p = stringFeedUntil(begin, "h", buf);
        CPPUNIT_ASSERT(p == begin + 1);
        CPPUNIT_ASSERT(buf == "");
    }

    void testUrlParse()
    {
        cstr_t szUrl;

        string scheme, domain, path;
        int port;

        szUrl = "scheme://domain:10/path?query_string#fragment_id";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "path?query_string#fragment_id");
        CPPUNIT_ASSERT(port == 10);

        szUrl = "scheme://domain/path?query_string#fragment_id";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "path?query_string#fragment_id");
        CPPUNIT_ASSERT(port == -1);

        szUrl = "scheme://domain";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "");
        CPPUNIT_ASSERT(port == -1);

        szUrl = "scheme://domain:12";
        CPPUNIT_ASSERT(urlParse(szUrl, scheme, domain, port, path));
        CPPUNIT_ASSERT(scheme == "scheme");
        CPPUNIT_ASSERT(domain == "domain");
        CPPUNIT_ASSERT(path == "");
        CPPUNIT_ASSERT(port == 12);
    }

    void testUriUnquote()
    {
        cstr_t cases[] = { "%E5%BC%A0%E6%AF%85", "artist.movie.1080p.xxx", 
            "", "%2%X1", };

        cstr_t results[] = { "张毅", "artist.movie.1080p.xxx", "", "%2%X1", };

        CPPUNIT_ASSERT(uriIsQuoted(cases[0]));
        for (int i = 0; i < CountOf(cases); i++)
        {
            string r = uriUnquote(cases[i]);
            CPPUNIT_ASSERT(strcmp(results[i], r.c_str()) == 0);
        }
    }

#endif // _CPPUNIT_TEST
