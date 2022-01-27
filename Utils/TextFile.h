#pragma once


class CTextFile
{
public:
    CTextFile();
    virtual ~CTextFile();

    // This will open all the file data to memory at one time.
    int open(cstr_t szFile, bool readMode, CHAR_ENCODING encoding);
    void close();

    bool readLine(string &strText);
    bool writeLine(cstr_t szText, size_t nLenText = -1);
    bool write(cstr_t szText, size_t nLenText = -1);

protected:
    CHAR_ENCODING           m_encoding;
    FILE                    *m_fp;

    string                  m_fileData;
    size_t                  m_offset;

    friend class CTestCaseCTextFile;

};
