#pragma once

#ifndef MPlayerEngine_MediaInputFile_h
#define MPlayerEngine_MediaInputFile_h


class MediaInputFile : public IMediaInput {
OBJ_REFERENCE_DECL
public:
    MediaInputFile();
    virtual ~MediaInputFile();

    virtual ResultCode open(cstr_t szSourceMedia);
    virtual uint32_t read(void *lpBuffer, uint32_t dwSize);
    virtual ResultCode seek(uint32_t dwOffset, int nOrigin = SEEK_SET);
    virtual ResultCode getSize(uint32_t &dwSize);
    virtual uint32_t getPos();

    virtual bool isEOF();
    virtual bool isError();

    virtual void close();

    virtual cstr_t getSource();

protected:
    FILE                        *m_fp;
    string                      m_strFile;

};

#endif // !defined(MPlayerEngine_MediaInputFile_h)
