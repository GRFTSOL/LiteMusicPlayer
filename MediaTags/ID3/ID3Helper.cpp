#include "ID3v2.h"
#include "ID3Helper.h"


void appendStrByEncodingAndBom(string &buff, cstr_t str, ID3v2EncType encType, CharEncodingType encoding) {
    if (encType == IET_UCS2) {
        buff += SZ_FE_UCS2;
        u16string strW;
        utf8ToUCS2(str, -1, strW);
        // ucs2EncodingReverse((WCHAR *)strW.data(), (int)strW.size());
        buff.append((const char *)strW.c_str(), (strW.size() + 1) * sizeof(WCHAR));
    } else if (encType == IET_UTF8) {
        //         buff += (char)0xEF;
        //         buff += (char)0xBB;
        //         buff += (char)0xBF;
        buff.append(str);
        buff.append(1, '\0');
    } else {
        // IET_ANSI
        string strAnsi;
        utf8ToMbcs(str, -1, strAnsi, encoding);
        buff.append(strAnsi.c_str(), strAnsi.size() + 1);
    }
}

// return the size of bytes used from buffer data
int copyAnsiStr(string &str, const char *data, int nLen) {
    str.resize(0);

    int nStrLen = (int)strlen_safe(data, nLen);
    str.append(data, nStrLen);
    if (nStrLen < nLen) {
        return nStrLen + 1;
    } else {
        return nStrLen;
    }
}

size_t wcslen_safe(const WCHAR * str, size_t maxLength) {
    size_t n = 0;
    while (n < maxLength && str[n]) {
        n++;
    }

    return n;
}

// return the size of bytes used from buffer data
int copyStrByEncodingAndBom(string &str, ID3v2EncType encType, const char *data, int len, CharEncodingType encoding) {
    int lenOrg = len;
    uint8_t *pbyData = (uint8_t *)data;

    str.resize(0);

    CharEncodingType srcEncoding = iD3v2EncTypeToCharEncoding(encType);

    // Determine the encoding type.
    if (pbyData[0] == 0xFF && pbyData[1] == 0xFE) {
        assert(encType == IET_UCS2);
        srcEncoding = ED_UNICODE;
        pbyData += 2;
        len -= 2;
    } else if (pbyData[0] == 0xFE && pbyData[1] == 0xFF) {
        assert(encType == IET_UCS2);
        srcEncoding = ED_UNICODE_BIG_ENDIAN;
        pbyData += 2;
        len -= 2;
    } else if (pbyData[0] == 0xEF && pbyData[1] == 0xBB && pbyData[2] == 0xBF) {
        srcEncoding = ED_UTF8;
        pbyData += 3;
        len -= 3;
    }

    // copy data
    int nCopyiedBytes = 0;
    if (srcEncoding == ED_UNICODE || srcEncoding == ED_UNICODE_BIG_ENDIAN) {
        int n = (int)wcslen_safe((WCHAR *)pbyData, len / sizeof(WCHAR));
        u16string temp;
        temp.append((WCHAR *)pbyData, n);
        if (srcEncoding == ED_UNICODE_BIG_ENDIAN) {
            ucs2EncodingReverse((WCHAR *)temp.c_str(), n);
        }
        ucs2ToUtf8((WCHAR *)temp.c_str(), n, str);

        nCopyiedBytes = n * sizeof(WCHAR);
        if (n != len / sizeof(WCHAR)) {
            nCopyiedBytes += sizeof(WCHAR);
        }
    } else if (srcEncoding == ED_UTF8) {
        int n = (int)strlen_safe((const char *)pbyData, len);
        str.assign((const char *)pbyData, n);
        nCopyiedBytes = n;
        if (n != len) {
            nCopyiedBytes++;
        }
    } else {
        int n = (int)strlen_safe((const char *)pbyData, len);
        mbcsToUtf8((const char *)pbyData, n, str, encoding);

        nCopyiedBytes = n;
        if (n != len) {
            nCopyiedBytes++;
        }
    }

    return lenOrg - len + nCopyiedBytes;
}

uint32_t syncBytesToUInt32(uint8_t *bytes) {
    uint32_t sum = 0;
    int last = 3;

    for (int i = 0; i <= last; i++) {
        sum |= (bytes[i] & 0x7f) << ((last - i) * 7);
    }

    return sum;
}

void syncBytesFromUInt32(uint32_t value, uint8_t *bytes) {
    int last = 4 - 1;
    for (int i = 0; i < 4; i++) {
        bytes[i] = uint8_t((value >> ((last - i) * 7)) & 0x7f);
    }
}

/**
 *  %11111111 111xxxxx
 *  and should be replaced with:
 *  %11111111 00000000 111xxxxx
 */
void synchDataDecode(string &data) {
    if (data.size() == 0) {
        return;
    }

    uint8_t *src = (uint8_t *)data.data(), *dst = src;
    uint8_t *end = (uint8_t *)data.data() + data.size() - 1;

    for (; src < end; dst++, src++) {
        *dst = *src;
        if (src[0] == 0xff && src[1] == 0) {
            // %11111111 00000000 xxxxxxxx ==> %11111111 xxxxxxxx
            src++;
        }
    }

    if (src < (uint8_t *)data.data() + data.size()) {
        *dst++ = *src++;
    }

    data.resize(size_t(dst - (uint8_t *)data.data()));
}

string synchDataEncode(const string &data) {
    string out;
    if (data.size() == 0) {
        return out;
    }

    out.resize(data.size() * 2);

    uint8_t *src = (uint8_t *)data.data(), *dst = (uint8_t *)out.data();
    uint8_t *end = (uint8_t *)data.data() + data.size() - 1;

    for (; src < end; dst++, src++) {
        *dst = *src;
        if (src[0] == 0xff) {
            if ((src[1] & 0xe0) == 0xe0 || src[1] == 0) {
                // %11111111 111xxxxx ==> %11111111 00000000 111xxxxx
                // %11111111 00000000 ==> %11111111 00000000 00000000
                *++dst = 0;
            }
        }
    }

    if (src < (uint8_t *)data.data() + data.size()) {
        *dst++ = *src++;
    }

    out.resize(size_t(dst - (uint8_t *)out.data()));
    return out;
}

uint32_t byteDataToUInt(uint8_t *byData, int nLen) {
    uint32_t sum = 0;
    int last;
    if (nLen > 4) {
        last = 3;
    } else {
        last = nLen - 1;
    }

    for(int i = 0; i <= last; i++) {
        sum |= byData[i] << ((last - i) * 8);
    }

    return sum;
}

void byteDataFromUInt(uint32_t value, uint8_t *byData, int nLen) {
    int last;
    last = nLen - 1;
    for(int i = 0; i < nLen; i++) {
        byData[i] = uint8_t((value >> ((last - i) * 8)) & 0xff);
    }
}

int fileDataReset(FILE *m_fp, long offset, long nSize, int nMask) {
    char buff[4096];

    memset(buff, nMask, sizeof(buff));

    if (fseek(m_fp, offset, SEEK_SET) != 0) {
        return ERR_SEEK_FILE;
    }

    for (long i = nSize; i > 0; i -= sizeof(buff)) {
        long nLen;

        if (i >= sizeof(buff)) {
            nLen = sizeof(buff);
        } else {
            nLen = i;
        }

        if (fwrite(buff, 1, nLen, m_fp) != nLen) {
            return ERR_WRITE_FILE;
        }
    }

    return ERR_OK;
}

int fileDataCmp(FILE *m_fp, long offset, const void *lpData, long nSize, int &nResult) {
    char buff[4096];
    char *pData = (char *)lpData;

    if (fseek(m_fp, offset, SEEK_SET) != 0) {
        return ERR_SEEK_FILE;
    }

    nResult = 0;
    for (long i = nSize; i > 0; i -= sizeof(buff)) {
        long nLen;

        if (i >= sizeof(buff)) {
            nLen = sizeof(buff);
        } else {
            nLen = i;
        }

        if (fread(buff, 1, nLen, m_fp) != nLen) {
            return ERR_READ_FILE;
        }
        nResult = memcmp(buff, pData, nLen);
        if (nResult != 0) {
            return ERR_OK;
        }
        pData += nLen;
    }

    return ERR_OK;
}

int fileMoveEndData(FILE *m_fp, long offset, long newOffset) {
    if (offset == newOffset) {
        return ERR_OK;
    }

    if (fseek(m_fp, 0, SEEK_END) != 0) {
        return ERR_SEEK_FILE;
    }

    auto fileSize = ftell(m_fp);
    if (fileSize >= 1024 * 1024 * 50) {
        // 文件太大，不支持修改
        return ERR_NO_MEM;
    }

    if (offset > fileSize) {
        // 移动的数据不能超出文件最大范围
        assert(offset <= fileSize);
        return ERR_FALSE;
    }

    if (offset < newOffset) {
        // 先确保磁盘空间足够
        if (fseek(m_fp, newOffset - 1, SEEK_SET) != 0) {
            return ERR_SEEK_FILE;
        }
        uint8_t buf[4] = {0};
        if (fwrite(buf, 1, 1, m_fp) != 1) {
            // 可能是磁盘空间不足，重新恢复原来的大小
            filetruncate(m_fp, fileSize);
            return ERR_WRITE_FILE;
        }
    }

    // Read all data.
    auto size = fileSize - offset;
    std::unique_ptr<char[]> buf(new char[size]);

    if (fseek(m_fp, offset, SEEK_SET) != 0) {
        return ERR_SEEK_FILE;
    }
    if (fread(buf.get(), 1, size, m_fp) != size) {
        return ERR_READ_FILE;
    }

    if (fseek(m_fp, newOffset, SEEK_SET) != 0) {
        return ERR_SEEK_FILE;
    }
    if (fwrite(buf.get(), 1, size, m_fp) != size) {
        return ERR_WRITE_FILE;
    }

    if (newOffset < offset) {
        // clip the file.
        filetruncate(m_fp, newOffset + size);
    }

    return ERR_OK;
}

/*
    test function for fileMoveEndData()
TEST(ID3Helper, FileMoveEndData)
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

#if UNIT_TEST

#include "../../TinyJS/utils/unittest.h"

TEST(ID3Helper, CopyAnsiStr) {
#define        SZ_TEXT_TEST "abc\0def\0"
    cstr_t text = SZ_TEXT_TEST;
    int lenText = CountOf(SZ_TEXT_TEST);
    int n;

    {
        string str;
        n = copyAnsiStr(str, text, lenText);
        ASSERT_TRUE(n == 4);
        ASSERT_TRUE(strcmp(str.c_str(), "abc") == 0);

        n = copyAnsiStr(str, text, 2);
        ASSERT_TRUE(n == 2);
        ASSERT_TRUE(strcmp(str.c_str(), "ab") == 0);
    }
}

TEST(ID3Helper, CopyStrByEncodingAndBom) {
#define        SZ_TEXT_TEST "abc\0def\0"

    cstr_t text = SZ_TEXT_TEST;
    int lenText = CountOf(SZ_TEXT_TEST);
    int n;
    u16string wStr;
    utf8ToUCS2(text, lenText, wStr);

    u16string wLyrics;
    wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
    wLyrics.append(wStr.c_str(), wStr.size());

    string utf8Lyrics;
    utf8Lyrics.append(SZ_FE_UTF8);
    utf8Lyrics.append(text, lenText);

    {
        string str;
        n = copyStrByEncodingAndBom(str, IET_ANSI, text, lenText);
        ASSERT_TRUE(n == 4);
        ASSERT_TRUE(strcmp(str.c_str(), "abc") == 0);

        n = copyStrByEncodingAndBom(str, IET_ANSI, text, 2);
        ASSERT_TRUE(n == 2);
        ASSERT_TRUE(strcmp(str.c_str(), "ab") == 0);

        n = copyStrByEncodingAndBom(str, IET_UCS2, (cstr_t)wLyrics.c_str(), (uint32_t)wLyrics.size() * sizeof(WCHAR));
        ASSERT_TRUE(n == (4 + 1) * sizeof(WCHAR));
        ASSERT_TRUE(strcmp(str.c_str(), "abc") == 0);

        n = copyStrByEncodingAndBom(str, IET_UCS2, (cstr_t)wLyrics.c_str(), 3 * sizeof(WCHAR));
        ASSERT_TRUE(n == (2 + 1) * sizeof(WCHAR));
        ASSERT_TRUE(strcmp(str.c_str(), "ab") == 0);

        n = copyStrByEncodingAndBom(str, IET_ANSI, utf8Lyrics.c_str(), (uint32_t)utf8Lyrics.size());
        ASSERT_TRUE(n == 4 + 3);
        ASSERT_TRUE(strcmp(str.c_str(), "abc") == 0);

        n = copyStrByEncodingAndBom(str, IET_ANSI, utf8Lyrics.c_str(), 2 + 3);
        ASSERT_TRUE(n == 2 + 3);
        ASSERT_TRUE(strcmp(str.c_str(), "ab") == 0);
    }
}

#endif
