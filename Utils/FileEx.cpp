#include "UtilsTypes.h"
#include "FileEx.h"
#include "Error.h"
#include "StrPrintf.h"
#include "BinaryStream.h"


CFileEx::CFileEx()
{
    m_fp = nullptr;
}

CFileEx::~CFileEx()
{
    close();
}

int CFileEx::open(cstr_t szFile, cstr_t szMode)
{
    close();

    m_fp = fopen(szFile, szMode);
    if (!m_fp)
    {
        setCustomErrorDesc(CStrPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    return ERR_OK;
}

void CFileEx::close()
{
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

int CFileEx::seek(int offset, int origin)
{
    if (fseek(m_fp, offset, origin) == 0)
        return ERR_OK;
    return ERR_SEEK_FILE;
}

long CFileEx::getPos()
{
    return ftell(m_fp);
}

bool CFileEx::isEOF()
{
    return feof(m_fp) != 0;
}

int CFileEx::readLine(string &line)
{
    return readTill(line, '\n');
}

int CFileEx::readTill(string &buf, char chTill)
{
    int c = fgetc(m_fp);
    if (c == EOF)
        return ERR_EOF;

    do
    {
        if (c == chTill)
            break;
        buf.append(1, (char)c);
        c = fgetc(m_fp);
    }
    while (c != EOF);

    return ERR_OK;
}

int CFileEx::readCount(void * buf, size_t count)
{
    size_t n = fread(buf, 1, count, m_fp);
    if (n != count)
        return ERR_READ_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeCount(void * buf, size_t count)
{
    size_t n = fwrite(buf, 1, count, m_fp);
    if (n != count)
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeByte(uint8_t v)
{
    size_t n = fwrite(&v, 1, 1, m_fp);
    if (n != 1)
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeInt16BE(int16_t v)
{
    uint8_t    buf[2];
    uint16ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeUInt16BE(uint16_t  v)
{
    uint8_t    buf[2];
    uint16ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeInt32BE(int32_t v)
{
    uint8_t    buf[4];
    uint32ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::writeUInt32BE(uint32_t v)
{
    uint8_t    buf[4];
    uint32ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_WRITE_FILE;
    else
        return ERR_OK;
}

int CFileEx::readByte(uint8_t &v)
{
    size_t n = fread(&v, 1, 1, m_fp);
    if (n != 1)
        return ERR_READ_FILE;
    else
        return ERR_OK;
}

int CFileEx::readInt16BE(int16_t &v)
{
    uint8_t buf[2];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_READ_FILE;

    v = (int)uint16FromBE(buf);
    return ERR_OK;
}

int CFileEx::readUInt16BE(uint16_t &v)
{
    uint8_t buf[2];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_READ_FILE;

    v = (int)uint16FromBE(buf);
    return ERR_OK;
}

int CFileEx::readInt32BE(int32_t &v)
{
    uint8_t buf[4];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_READ_FILE;

    v = (int)uint32FromBE(buf);
    return ERR_OK;
}

int CFileEx::readUInt32BE(uint32_t &v)
{
    uint8_t buf[4];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf))
        return ERR_READ_FILE;

    v = (int)uint32FromBE(buf);
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

#ifdef _CPPUNIT_TEST

IMPLEMENT_CPPUNIT_TEST_REG(CFileEx)

class CTestCaseCFileEx : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCFileEx);
    CPPUNIT_TEST(testReadWrite);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testReadWrite()
    {
        CFileEx    f;
        string file = dirStringJoin(getUnitTestFolder().c_str(), "test_FileEx.bin");

        // write
        int n = f.open(file.c_str(), "wb");
        CPPUNIT_ASSERT(n == ERR_OK);

        size_t count = f.write("a", 1);
        CPPUNIT_ASSERT(count == 1);

        n = f.writeByte(1);
        CPPUNIT_ASSERT(n == ERR_OK);

        n = f.writeInt16BE(258);
        CPPUNIT_ASSERT(n == ERR_OK);

        n = f.writeInt16BE((uint16_t)65535);
        CPPUNIT_ASSERT(n == ERR_OK);

        n = f.writeInt32BE(3688823);
        CPPUNIT_ASSERT(n == ERR_OK);

        n = f.writeUInt32BE(0xFFFFFFFF);
        CPPUNIT_ASSERT(n == ERR_OK);

        n = f.writeCount("ab", 2);
        CPPUNIT_ASSERT(n == ERR_OK);

        const char *s = "dde\r\n";
        n = f.write(s, strlen(s));

        f.close();


        // read
        n = f.open(file.c_str(), "rb");
        CPPUNIT_ASSERT(n == ERR_OK);

        uint8_t buf[64];
        count = f.read(buf, 1);
        CPPUNIT_ASSERT(count == 1);
        CPPUNIT_ASSERT(buf[0] == 'a');

        uint8_t vb;
        n = f.readByte(vb);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(vb == 1);

        short vs;
        n = f.readInt16BE(vs);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(vs == 258);
        n = f.readInt16BE(vs);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT((uint16_t)vs == 65535);

        int vn;
        n = f.readInt32BE(vn);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(vn == 3688823);

        uint32_t vu;
        n = f.readUInt32BE(vu);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(vu == 0xFFFFFFFF);

        n = f.readCount(buf, 2);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(buf[0] == 'a' && buf[1] == 'b');

        string str;
        n = f.readLine(str);
        CPPUNIT_ASSERT(n == ERR_OK);
        CPPUNIT_ASSERT(str == "dde\r");

        f.close();
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCFileEx);

#endif // _CPPUNIT_TEST
