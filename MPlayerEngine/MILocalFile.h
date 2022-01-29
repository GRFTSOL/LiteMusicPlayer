// MILocalFile.h: interface for the CMILocalFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_)
#define AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMILocalFile : public IMediaInput  
{
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
    FILE        *m_fp;
    string        m_strFile;

};

#endif // !defined(AFX_MILOCALFILE_H__1717B24F_3838_48AE_A77E_8014439E8535__INCLUDED_)
