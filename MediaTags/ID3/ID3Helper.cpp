// HelperFun.cpp: implementation of the CHelperFun class.
//
//////////////////////////////////////////////////////////////////////

#include "ID3v2.h"
#include "ID3Helper.h"


void appendStrByEncodingAndBom(string &buff, cstr_t str, ID3v2EncType encType, CharEncodingType encoding)
{
    if (encType == IET_UCS2LE_BOM)
    {
        buff += SZ_FE_UCS2;
        u16string    strW;
        utf8ToUCS2(str, -1, strW);
        ucs2EncodingReverse((WCHAR *)strW.data(), strW.size());
        buff.append((const char *)strW.c_str(), (strW.size() + 1) * sizeof(WCHAR));
    }
    else if (encType == IET_UCS2BE_NO_BOM)
    {
        u16string    strW;
        utf8ToUCS2(str, -1, strW);
        buff.append((const char *)strW.c_str(), (strW.size() + 1) * sizeof(WCHAR));
    }
    else if (encType == IET_UTF8)
    {
        //         buff += (char)0xEF;
        //         buff += (char)0xBB;
        //         buff += (char)0xBF;
        buff.append(str);
        buff.append(1, '\0');
    }
    else
    {
        // IET_ANSI
        string    strAnsi;
        utf8ToMbcs(str, -1, strAnsi, encoding);
        buff.append(strAnsi.c_str(), strAnsi.size() + 1);
    }
}

// return the size of bytes used from buffer data
int copyAnsiStr(string &str, const char *data, int nLen)
{
    str.resize(0);

    int nStrLen = strlen_safe(data, nLen);
    str.append(data, nStrLen);
    if (nStrLen < nLen)
        return nStrLen + 1;
    else
        return nStrLen;
}

size_t wcslen_safe(const WCHAR * str, size_t maxLength)
{
	size_t n = 0;
	while (n < maxLength && str[n])
		n++;

	return n;
}

// return the size of bytes used from buffer data
int copyStrByEncodingAndBom(string &str, ID3v2EncType encType, const char *data, int len, CharEncodingType encoding)
{
    int        lenOrg = len;
    uint8_t *pbyData = (uint8_t *)data;

    str.resize(0);

    // Determine the encoding type.
    if (pbyData[0] == 0xFF && pbyData[1] == 0xFE)
    {
        assert(encType == IET_UCS2LE_BOM);
        encType = IET_UCS2LE_BOM;
        pbyData += 2;
        len -= 2;
    }
    else if (pbyData[0] == 0xFE && pbyData[1] == 0xFF)
    {
        assert(encType == IET_UCS2BE_NO_BOM);
        encType = IET_UCS2BE_NO_BOM;
        pbyData += 2;
        len -= 2;
    }
    else if (pbyData[0] == 0xEF && pbyData[1] == 0xBB && pbyData[2] == 0xBF)
    {
        encType = IET_UTF8;
        pbyData += 3;
        len -= 3;
    }

    // copy data
    int        nCopyiedBytes = 0;
    if (encType == IET_UCS2LE_BOM || encType == IET_UCS2BE_NO_BOM)
    {
        int n = wcslen_safe((WCHAR *)pbyData, len / sizeof(WCHAR));
        u16string temp;
        temp.append((WCHAR *)pbyData, n);
        if (encType == IET_UCS2BE_NO_BOM)
            ucs2EncodingReverse((WCHAR *)temp.c_str(), n);
        ucs2ToUtf8((WCHAR *)temp.c_str(), n, str);

        nCopyiedBytes = n * sizeof(WCHAR);
        if (n != len / sizeof(WCHAR))
            nCopyiedBytes += sizeof(WCHAR);
    }
    else if (encType == IET_UTF8)
    {
        int n = strlen_safe((const char *)pbyData, len);
        str.assign((const char *)pbyData, n);
        nCopyiedBytes = n;
        if (n != len)
            nCopyiedBytes++;
    }
    else
    {
        int n = strlen_safe((const char *)pbyData, len);
        mbcsToUtf8((const char *)pbyData, n, str, encoding);

        nCopyiedBytes = n;
        if (n != len)
            nCopyiedBytes++;
    }

    return lenOrg - len + nCopyiedBytes;
}

uint32_t synchDataToUInt(uint8_t *byData, int nLen)
{
    uint32_t    sum = 0;
    int        last;
    if (nLen > 4)
        last = 3;
    else
        last = nLen - 1;

    for(int i = 0; i <= last; i++)
        sum |= (byData[i] & 0x7f) << ((last - i) * 7);

    return sum;
}

void synchDataFromUInt(uint32_t value, uint8_t *byData, int nLen)
{
    int        last;
    last = nLen - 1;
    for(int i = 0; i < nLen; i++)
        byData[i] = uint8_t((value >> ((last - i) * 7)) & 0x7f);

}

uint32_t byteDataToUInt(uint8_t *byData, int nLen)
{
    uint32_t    sum = 0;
    int        last;
    if (nLen > 4)
        last = 3;
    else
        last = nLen - 1;

    for(int i = 0; i <= last; i++)
        sum |= byData[i] << ((last - i) * 8);

    return sum;
}

void byteDataFromUInt(uint32_t value, uint8_t *byData, int nLen)
{
    int        last;
    last = nLen - 1;
    for(int i = 0; i < nLen; i++)
        byData[i] = uint8_t((value >> ((last - i) * 8)) & 0xff);
}

int fileDataReset(FILE *m_fp, int nOffset, int nSize, int nMask)
{
    char    buff[4096];

    memset(buff, nMask, sizeof(buff));

    if (fseek(m_fp, nOffset, SEEK_SET) != 0)
        return ERR_SEEK_FILE;

    int        nLen;

    for (int i = nSize; i > 0; i -= sizeof(buff))
    {
        if (i >= sizeof(buff))
            nLen = sizeof(buff);
        else
            nLen = i;

        if (fwrite(buff, 1, nLen, m_fp) != nLen)
        {
            return ERR_WRITE_FILE;
        }
    }

    return ERR_OK;
}

int fileDataCmp(FILE *m_fp, int nOffset, const void *lpData, int nSize, int &nResult)
{
    char    buff[4096];
    char    *pData = (char *)lpData;

    if (fseek(m_fp, nOffset, SEEK_SET) != 0)
        return ERR_SEEK_FILE;

    int        nLen;

    nResult = 0;
    for (int i = nSize; i > 0; i -= sizeof(buff))
    {
        if (i >= sizeof(buff))
            nLen = sizeof(buff);
        else
            nLen = i;

        if (fread(buff, 1, nLen, m_fp) != nLen)
        {
            return ERR_READ_FILE;
        }
        nResult = memcmp(buff, pData, nLen);
        if (nResult != 0)
            return ERR_OK;
        pData += nLen;
    }

    return ERR_OK;
}

int fileAppendData(FILE *m_fp, int nAppend, int nMask)
{
    char    buff[4096];
    int        nLen;

    memset(buff, nMask, sizeof(buff));

    if (fseek(m_fp, 0, SEEK_END) != 0)
        return ERR_SEEK_FILE;

    long orgLength = ftell(m_fp);
    if (orgLength == -1)
        return ERR_READ_FILE;

    for (int i = nAppend; i > 0; i -= sizeof(buff))
    {
        if (i >= sizeof(buff))
            nLen = sizeof(buff);
        else
            nLen = i;

        if (fwrite(buff, 1, nLen, m_fp) != nLen)
        {
            // write error!
            filetruncate(m_fp, orgLength);

            return ERR_WRITE_FILE;
        }
    }

    return ERR_OK;
}

int fileMoveEndData(FILE *m_fp, int nOffset, int nNewOffset)
{
    const int BUF_SIZE = 1024 * 32;
    std::unique_ptr<char[]> buff(new char[BUF_SIZE]);

    int            nLen, nPos, nFileLen, nNewAppend;
    int            nRet;

    if (fseek(m_fp, 0, SEEK_END) != 0)
        return ERR_SEEK_FILE;

    nFileLen = ftell(m_fp);

    if (nOffset >= nFileLen)
    {
        assert(nOffset < nFileLen);
        return ERR_FALSE;
    }

    if (nNewOffset == nOffset)
        return ERR_OK;
    else if (nNewOffset > nOffset)
    {
        nRet = fileAppendData(m_fp, nNewOffset - nOffset);
        if (nRet != ERR_OK)
            return nRet;

        int        nLastEndPos;

        nNewAppend = nNewOffset - nOffset;
        nLastEndPos = nFileLen;

        // from end to move data
        //     ---oooo
        //     ----oooo
        while (nLastEndPos > nOffset)
        {
            nPos = nLastEndPos - BUF_SIZE;
            if (nPos < nOffset)
                nPos = nOffset;
            nLen = nLastEndPos - nPos;

            if (fseek(m_fp, nPos, SEEK_SET) != 0)
                return ERR_SEEK_FILE;
            if (fread(buff.get(), 1, nLen, m_fp) != nLen)
                return ERR_READ_FILE;
            if (fseek(m_fp, nPos + nNewAppend, SEEK_SET) != 0)
                return ERR_SEEK_FILE;
            if (fwrite(buff.get(), 1, nLen, m_fp) != nLen)
                return ERR_WRITE_FILE;

            nLastEndPos = nPos;
        }
    }
    else
    {
        // from begin to move data
        //     ---oooo
        //     --oooo
        int        nNextPos;

        nNewAppend = nOffset - nNewOffset;
        nPos = nOffset;
        while (nPos < nFileLen)
        {
            nNextPos = nPos + BUF_SIZE;
            if (nNextPos > nFileLen)
                nNextPos = nFileLen;
            nLen = nNextPos - nPos;

            if (fseek(m_fp, nPos, SEEK_SET) != 0)
                return ERR_SEEK_FILE;
            if (fread(buff.get(), 1, nLen, m_fp) != nLen)
                return ERR_READ_FILE;
            if (fseek(m_fp, nPos - nNewAppend, SEEK_SET) != 0)
                return ERR_SEEK_FILE;
            if (fwrite(buff.get(), 1, nLen, m_fp) != nLen)
                return ERR_WRITE_FILE;

            nPos = nNextPos;
        }
        // clip the file.
        filetruncate(m_fp, nFileLen - nNewAppend);
    }

    return ERR_OK;
}

/*
    test function for fileMoveEndData()
void testFileMoveEndData()
{
    // fileMoveEndData
    FILE        *fp;
    fp = fopen("c:\\1.txt", "wb");
    if (!fp)
        return;

    for (int i = 0; i < 100; i++)
    {
        fprintf(fp, "%d", i % 10);
    }
    fclose(fp);

    fp = fopen("c:\\1.txt", "r+b");
    if (!fp)
        return;
    fileMoveEndData(fp, 90, 89);
    fclose(fp);
}
*/

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(ID3_HELPER)

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseLrcTag : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseLrcTag);
    CPPUNIT_TEST(testCopyAnsiStr);
    CPPUNIT_TEST(testCopyStrByEncodingAndBom);
    CPPUNIT_TEST_SUITE_END();

protected:

public:
    void setUp()
    {
    }
    void tearDown() 
    {
    }

protected:
    void testCopyAnsiStr()
    {
#define        SZ_TEXT_TEST    "abc\0def\0"
        cstr_t        text = SZ_TEXT_TEST;
        int            lenText = CountOf(SZ_TEXT_TEST);
        int            n;

        {
            string        str;
            n = copyAnsiStr(str, text, lenText);
            CPPUNIT_ASSERT(n == 4);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

            n = copyAnsiStr(str, text, 2);
            CPPUNIT_ASSERT(n == 2);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);
        }
    }

    void testCopyStrByEncodingAndBom()
    {
#define        SZ_TEXT_TEST    "abc\0def\0"
        cstr_t        text = SZ_TEXT_TEST;
        int            lenText = CountOf(SZ_TEXT_TEST);
        int            n;
        string    wStr;
        utf8ToUCS2(text, lenText, wStr);

        string    wLyrics;
        wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
        wLyrics.append(wStr.c_str(), wStr.size());

        string    wLyricsBe;
        wLyricsBe.append(wStr.c_str(), wStr.size());
        uCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

        string    utf8Lyrics;
        utf8Lyrics.append(SZ_FE_UTF8);
        utf8Lyrics.append(text, lenText);

        {
            string        str;
            n = copyStrByEncodingAndBom(str, IET_ANSI, text, lenText);
            CPPUNIT_ASSERT(n == 4);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

            n = copyStrByEncodingAndBom(str, IET_ANSI, text, 2);
            CPPUNIT_ASSERT(n == 2);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);

            n = copyStrByEncodingAndBom(str, IET_UCS2LE_BOM, (cstr_t)wLyrics.c_str(), wLyrics.size() * sizeof(WCHAR));
            CPPUNIT_ASSERT(n == (4 + 1) * sizeof(WCHAR));
            CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

            n = copyStrByEncodingAndBom(str, IET_UCS2LE_BOM, (cstr_t)wLyrics.c_str(), 3 * sizeof(WCHAR));
            CPPUNIT_ASSERT(n == (2 + 1) * sizeof(WCHAR));
            CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);

            n = copyStrByEncodingAndBom(str, IET_UCS2BE_NO_BOM, (cstr_t)wLyricsBe.c_str(), wLyricsBe.size() * sizeof(WCHAR));
            CPPUNIT_ASSERT(n == 4 * sizeof(WCHAR));
            CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

            n = copyStrByEncodingAndBom(str, IET_UCS2BE_NO_BOM, (cstr_t)wLyricsBe.c_str(), 2 * sizeof(WCHAR));
            CPPUNIT_ASSERT(n == 2 * sizeof(WCHAR));
            CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);

            n = copyStrByEncodingAndBom(str, IET_ANSI, utf8Lyrics.c_str(), utf8Lyrics.size());
            CPPUNIT_ASSERT(n == 4 + 3);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "abc") == 0);

            n = copyStrByEncodingAndBom(str, IET_ANSI, utf8Lyrics.c_str(), 2 + 3);
            CPPUNIT_ASSERT(n == 2 + 3);
            CPPUNIT_ASSERT(strcmp(str.c_str(), "ab") == 0);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcTag);

#endif // _CPPUNIT_TEST
