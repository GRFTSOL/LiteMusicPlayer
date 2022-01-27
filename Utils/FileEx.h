#pragma once

class CFileEx
{
public:
    CFileEx();
    virtual ~CFileEx();

    int open(cstr_t szFile, cstr_t szMode);
    void close();

    int seek(int offset, int origin = SEEK_CUR);
    long getPos();

    bool isEOF();

    size_t read(void * buf, size_t count) { return fread(buf, 1, count, m_fp); }
    size_t write(const void *buf, size_t count) { return fwrite(buf, 1, count, m_fp); }

    int readLine(string &line);
    int readTill(string &buf, char chTill);

    int readCount(void * buf, size_t count);
    int writeCount(void * buf, size_t count);

    int writeByte(uint8_t v);
    int writeInt16BE(int16_t v);
    int writeUInt16BE(uint16_t v);
    int writeInt32BE(int32_t v);
    int writeUInt32BE(uint32_t v);

    int readByte(uint8_t &v);
    int readInt16BE(int16_t &v);
    int readUInt16BE(uint16_t &v);
    int readInt32BE(int32_t &v);
    int readUInt32BE(uint32_t &v);

protected:
    FILE            *m_fp;

};
