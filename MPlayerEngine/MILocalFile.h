#pragma once

#ifndef MPlayerEngine_MILocalFile_h
#define MPlayerEngine_MILocalFile_h


class CMILocalFile : public IMediaInput {
OBJ_REFERENCE_DECL
public:
    CMILocalFile();
    virtual ~CMILocalFile();

    virtual MLRESULT open(cstr_t szSourceMedia);
    virtual uint32_t read(void *lpBuffer, uint32_t dwSize);
    virtual MLRESULT seek(uint32_t dwOffset, int nOrigin = SEEK_SET);
    virtual MLRESULT getSize(uint32_t &dwSize);
    virtual uint32_t getPos();

    virtual bool isEOF();
    virtual bool isError();

    virtual void close();

    virtual cstr_t getSource();

protected:
    FILE                        *m_fp;
    string                      m_strFile;

};

#endif // !defined(MPlayerEngine_MILocalFile_h)
