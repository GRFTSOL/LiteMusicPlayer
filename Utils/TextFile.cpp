#include "Utils.h"
#include "TextFile.h"


CTextFile::CTextFile() {
    m_offset = 0;
    m_fp = nullptr;
    m_encoding = ED_SYSDEF;
}

CTextFile::CTextFile(const void *data, size_t len) : CTextFile() {
    m_fileData.assign((cstr_t)data, len);
}

CTextFile::~CTextFile() {
    close();
}

int CTextFile::open(cstr_t szFile, bool writeMode, CharEncodingType encoding) {
    close();

    m_fp = fopen(szFile, writeMode ? "wb" : "rb");
    if (!m_fp) {
        setCustomErrorDesc(stringPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    m_offset = 0;
    m_encoding = encoding;
    m_fileData.clear();

    cstr_t bom = getFileEncodingBom(encoding);
    if (writeMode) {
        fwrite(bom, 1, strlen(bom), m_fp);
    } else {
        string data;
        if (readFile(szFile, data)) {
            if (strncmp(bom, data.c_str(), strlen(bom)) == 0) {
                // 去掉 BOM
                data.erase(data.begin(), data.begin() + strlen(bom));
            }
            mbcsToUtf8(data.c_str(), (int)data.size(), m_fileData, m_encoding);
        }
    }

    return ERR_OK;
}

void CTextFile::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_fileData.clear();
}

bool CTextFile::readLine(string &strText) {
    cstr_t begin = m_fileData.c_str() + m_offset;
    cstr_t end = m_fileData.c_str() + m_fileData.size();
    cstr_t p = begin;

    strText.clear();

    while (p < end) {
        if (*p == '\r' || *p == '\n') {
            // Found new line position
            strText.append(begin, (size_t)(p - begin));
            if (*p == '\r' && p[1] == '\n') {
                p++;
            }
            p++;

            m_offset = (size_t)(p - m_fileData.c_str());
            return true;
        }
        p++;
    }

    // To the end of file
    if (m_offset == m_fileData.size()) {
        return false;
    }

    strText.assign(begin, (size_t)(end - begin));
    m_offset = m_fileData.size();

    return true;
}

bool CTextFile::writeLine(cstr_t szText, size_t nLenText) {
    if (!write(szText, nLenText)) {
        return false;
    }

    // write new line
    return write("\r\n");
}

bool CTextFile::write(cstr_t szText, size_t nLenText) {
    if (nLenText == -1) {
        nLenText = strlen(szText);
    }

    size_t nWritten = 0;
    if (m_encoding == ED_UNICODE || m_encoding == ED_UNICODE_BIG_ENDIAN) {
        utf16string strUCS2;
        utf8ToUCS2(szText, (int)nLenText, strUCS2);
        if (m_encoding == ED_UNICODE) {
            //ucs2EncodingReverse((WCHAR *)strUCS2.c_str(), (int)strUCS2.size());
        }

        nWritten = fwrite(strUCS2.c_str(), sizeof(WCHAR), strUCS2.size(), m_fp);
        if (nWritten != strUCS2.size()) {
            return false;
        }
    } else if (m_encoding == ED_UTF8) {
        nWritten = fwrite(szText, sizeof(char), nLenText, m_fp);
        if (nWritten != nLenText) {
            return false;
        }
    } else {
        string strMbcs;
        utf8ToMbcs(szText, (int)nLenText, strMbcs, m_encoding);

        nWritten = fwrite(strMbcs.c_str(), sizeof(char), strMbcs.size(), m_fp);
        if (nWritten != strMbcs.size()) {
            return false;
        }
    }

    return true;
}

#if UNIT_TEST

#include "utils/unittest.h"


TEST(TextFile, ReadLine) {
    auto str = "l1\nl2\rl3\r\nl4";
    CTextFile file(str, strlen(str));

    string line;

    ASSERT_TRUE(file.readLine(line));
    ASSERT_TRUE(line == "l1");

    ASSERT_TRUE(file.readLine(line));
    ASSERT_TRUE(line == "l2");

    ASSERT_TRUE(file.readLine(line));
    ASSERT_TRUE(line == "l3");

    ASSERT_TRUE(file.readLine(line));
    ASSERT_TRUE(line == "l4");
}

TEST(TextFile, All) {
    CTextFile file;

    string fileName = getUnittestTempDir() + "testTextFile.txt";

    CharEncodingType encodings[] = { ED_UTF8, ED_UNICODE, ED_SYSDEF };

    for (int i = 0; i < CountOf(encodings); i++) {
        int nRet = file.open(fileName.c_str(), true, encodings[i]);
        ASSERT_TRUE(nRet == ERR_OK);

        ASSERT_TRUE(file.writeLine("l1"));
        ASSERT_TRUE(file.writeLine("l2"));
        ASSERT_TRUE(file.writeLine("l3"));

        file.close();

        nRet = file.open(fileName.c_str(), false, encodings[i]);
        ASSERT_TRUE(nRet == ERR_OK);

        string line;

        ASSERT_TRUE(file.readLine(line));
        ASSERT_EQ(line, "l1");

        ASSERT_TRUE(file.readLine(line));
        ASSERT_TRUE(line == "l2");

        ASSERT_TRUE(file.readLine(line));
        ASSERT_TRUE(line == "l3");
    }

    deleteFile(fileName.c_str());
}

#endif
