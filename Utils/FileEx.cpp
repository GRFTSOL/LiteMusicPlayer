#include "Utils.h"
#include "FileEx.h"
#include "Error.h"


CFileEx::CFileEx() {
    m_fp = nullptr;
}

CFileEx::~CFileEx() {
    close();
}

int CFileEx::open(cstr_t szFile, cstr_t szMode) {
    close();

    m_fp = fopen(szFile, szMode);
    if (!m_fp) {
        setCustomErrorDesc(stringPrintf("%s: %s", (cstr_t)OSError(), szFile).c_str());
        return ERR_CUSTOM_ERROR;
    }

    return ERR_OK;
}

void CFileEx::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = nullptr;
    }
}

int CFileEx::seek(int offset, int origin) {
    if (fseek(m_fp, offset, origin) == 0) {
        return ERR_OK;
    }
    return ERR_SEEK_FILE;
}

long CFileEx::getPos() {
    return ftell(m_fp);
}

bool CFileEx::isEOF() {
    return feof(m_fp) != 0;
}

int CFileEx::readLine(string &line) {
    return readTill(line, '\n');
}

int CFileEx::readTill(string &buf, char chTill) {
    int c = fgetc(m_fp);
    if (c == EOF) {
        return ERR_EOF;
    }

    do {
        if (c == chTill) {
            break;
        }
        buf.append(1, (char)c);
        c = fgetc(m_fp);
    }
    while (c != EOF);

    return ERR_OK;
}

int CFileEx::readCount(void * buf, size_t count) {
    size_t n = fread(buf, 1, count, m_fp);
    if (n != count) {
        return ERR_READ_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeCount(const void * buf, size_t count) {
    size_t n = fwrite(buf, 1, count, m_fp);
    if (n != count) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeByte(uint8_t v) {
    size_t n = fwrite(&v, 1, 1, m_fp);
    if (n != 1) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeInt16BE(int16_t v) {
    uint8_t buf[2];
    uint16ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeUInt16BE(uint16_t  v) {
    uint8_t buf[2];
    uint16ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeInt32BE(int32_t v) {
    uint8_t buf[4];
    uint32ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::writeUInt32BE(uint32_t v) {
    uint8_t buf[4];
    uint32ToBE(v, buf);

    size_t n = fwrite(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_WRITE_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::readByte(uint8_t &v) {
    size_t n = fread(&v, 1, 1, m_fp);
    if (n != 1) {
        return ERR_READ_FILE;
    } else {
        return ERR_OK;
    }
}

int CFileEx::readInt16BE(int16_t &v) {
    uint8_t buf[2];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_READ_FILE;
    }

    v = (int)uint16FromBE(buf);
    return ERR_OK;
}

int CFileEx::readUInt16BE(uint16_t &v) {
    uint8_t buf[2];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_READ_FILE;
    }

    v = (int)uint16FromBE(buf);
    return ERR_OK;
}

int CFileEx::readInt32BE(int32_t &v) {
    uint8_t buf[4];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_READ_FILE;
    }

    v = (int)uint32FromBE(buf);
    return ERR_OK;
}

int CFileEx::readUInt32BE(uint32_t &v) {
    uint8_t buf[4];

    size_t n = fread(&buf, 1, CountOf(buf), m_fp);
    if (n != CountOf(buf)) {
        return ERR_READ_FILE;
    }

    v = (int)uint32FromBE(buf);
    return ERR_OK;
}

#if UNIT_TEST

#include "utils/unittest.h"


TEST(FileEx, ReadWrite) {
    CFileEx f;
    string file = dirStringJoin(getUnittestTempDir().c_str(), "test_FileEx.bin");

    // write
    int n = f.open(file.c_str(), "wb");
    ASSERT_TRUE(n == ERR_OK);

    size_t count = f.write("a", 1);
    ASSERT_TRUE(count == 1);

    n = f.writeByte(1);
    ASSERT_TRUE(n == ERR_OK);

    n = f.writeInt16BE(258);
    ASSERT_TRUE(n == ERR_OK);

    n = f.writeInt16BE((uint16_t)65535);
    ASSERT_TRUE(n == ERR_OK);

    n = f.writeInt32BE(3688823);
    ASSERT_TRUE(n == ERR_OK);

    n = f.writeUInt32BE(0xFFFFFFFF);
    ASSERT_TRUE(n == ERR_OK);

    n = f.writeCount("ab", 2);
    ASSERT_TRUE(n == ERR_OK);

    const char *s = "dde\r\n";
    n = (int)f.write(s, strlen(s));

    f.close();


    // read
    n = f.open(file.c_str(), "rb");
    ASSERT_TRUE(n == ERR_OK);

    uint8_t buf[64];
    count = f.read(buf, 1);
    ASSERT_TRUE(count == 1);
    ASSERT_TRUE(buf[0] == 'a');

    uint8_t vb;
    n = f.readByte(vb);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(vb == 1);

    short vs;
    n = f.readInt16BE(vs);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(vs == 258);
    n = f.readInt16BE(vs);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE((uint16_t)vs == 65535);

    int vn;
    n = f.readInt32BE(vn);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(vn == 3688823);

    uint32_t vu;
    n = f.readUInt32BE(vu);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(vu == 0xFFFFFFFF);

    n = f.readCount(buf, 2);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(buf[0] == 'a' && buf[1] == 'b');

    string str;
    n = f.readLine(str);
    ASSERT_TRUE(n == ERR_OK);
    ASSERT_TRUE(str == "dde\r");

    f.close();

    deleteFile(file.c_str());
}

#endif
