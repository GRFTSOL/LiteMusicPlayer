#include "Utils.h"
#include "TextFile.h"

CTextFile::CTextFile()
{
    m_offset = 0;
    m_fp = nullptr;
    m_encoding = ED_SYSDEF;
}

CTextFile::~CTextFile()
{
    close();
}

int CTextFile::open(cstr_t szFile, bool readMode, CHAR_ENCODING encoding)
{
    close();

    cstr_t            szMode = "rb";

    if (!readMode)
        szMode = "wb";

    m_fp = fopen(szFile, szMode);
    if (!m_fp)
    {
        setCustomErrorDesc(CStrPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    m_offset = 0;
    m_encoding =  encoding;
    m_fileData.clear();

    if (readMode) {
        string data;
        if (readFile(szFile, data)) {
            mbcsToUtf8(data.c_str(), data.size(), m_fileData, m_encoding);
        }
    } else {
        cstr_t bom = getFileEncodingBom(encoding);
        fwrite(bom, 1, strlen(bom), m_fp);
    }

    return ERR_OK;
}

void CTextFile::close()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }

    m_fileData.clear();
}

bool CTextFile::readLine(string &strText)
{
    cstr_t begin = m_fileData.c_str() + m_offset;
    cstr_t end = m_fileData.c_str() + m_fileData.size();
    cstr_t p = begin;

    strText.clear();

    while (p < end)
    {
        if (*p == '\r' || *p == '\n')
        {
            // Found new line position
            strText.append(begin, (size_t)(p - begin));
            if (*p == '\r' && p[1] == '\n')
                p++;
            p++;

            m_offset = (size_t)(p - m_fileData.c_str());
            return true;
        }
        p++;
    }

    // To the end of file
    if (m_offset == m_fileData.size())
        return false;

    strText.append(begin);
    m_offset = m_fileData.size();

    return true;
}

bool CTextFile::writeLine(cstr_t szText, size_t nLenText)
{
    if (!write(szText, nLenText))
        return false;

    // write new line
    return write("\r\n");
}

bool CTextFile::write(cstr_t szText, size_t nLenText)
{
    if (nLenText == -1)
        nLenText = strlen(szText);

    size_t nWritten = 0;
    if (m_encoding == ED_UNICODE || m_encoding == ED_UNICODE_BIG_ENDIAN)
    {
        wstring strUCS2;
        utf8ToUCS2(szText, (int)nLenText, strUCS2);
        if (m_encoding == ED_UNICODE_BIG_ENDIAN) {
            ucs2EncodingReverse((WCHAR *)strUCS2.c_str(), strUCS2.size());
        }

        nWritten = fwrite(strUCS2.c_str(), sizeof(WCHAR), strUCS2.size(), m_fp);
        if (nWritten != strUCS2.size())
            return false;
    }
    else if (m_encoding == ED_UTF8)
    {
        nWritten = fwrite(szText, sizeof(char), nLenText, m_fp);
        if (nWritten != nLenText)
            return false;
    }
    else
    {
        string strMbcs;
        utf8ToMbcs(szText, nLenText, strMbcs, m_encoding);

        nWritten = fwrite(strMbcs.c_str(), sizeof(char), strMbcs.size(), m_fp);
        if (nWritten != strMbcs.size())
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CTextFile)

class CTestCaseCTextFile : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCTextFile);
    CPPUNIT_TEST(testReadLine);
    CPPUNIT_TEST(testAll);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testReadLine()
    {
        CTextFile    file;

        file.m_fileData = "l1\nl2\rl3\r\nl4";

        string line;

        CPPUNIT_ASSERT(file.readLine(line));
        CPPUNIT_ASSERT(line == "l1");

        CPPUNIT_ASSERT(file.readLine(line));
        CPPUNIT_ASSERT(line == "l2");

        CPPUNIT_ASSERT(file.readLine(line));
        CPPUNIT_ASSERT(line == "l3");

        CPPUNIT_ASSERT(file.readLine(line));
        CPPUNIT_ASSERT(line == "l4");
    }

    void testAll()
    {
        CTextFile    file;

        string szFile = getUnitTestFolder() + "testTextFile.txt";

        CHAR_ENCODING encodings[] = { ED_UTF8, ED_UNICODE, ED_SYSDEF };

        for (int i = 0; i < CountOf(encodings); i++)
        {
            int nRet = file.open(szFile.c_str(), CTextFile::OM_WRITE, encodings[i]);
            CPPUNIT_ASSERT(nRet == ERR_OK);

            CPPUNIT_ASSERT(file.writeLine("l1"));
            CPPUNIT_ASSERT(file.writeLine("l2"));
            CPPUNIT_ASSERT(file.writeLine("l3"));

            file.close();

            nRet = file.open(szFile.c_str(), CTextFile::OM_READ, ED_SYSDEF);
            CPPUNIT_ASSERT(nRet == ERR_OK);

            string line;

            CPPUNIT_ASSERT(file.readLine(line));
            CPPUNIT_ASSERT(line == "l1");

            CPPUNIT_ASSERT(file.readLine(line));
            CPPUNIT_ASSERT(line == "l2");

            CPPUNIT_ASSERT(file.readLine(line));
            CPPUNIT_ASSERT(line == "l3");
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCTextFile);

#endif // _CPPUNIT_TEST
