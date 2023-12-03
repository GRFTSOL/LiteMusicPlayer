#ifndef MediaTags_ID3_ID3Helper_h
#define MediaTags_ID3_ID3Helper_h

#pragma once


// append text included 0x0 and bom(for ucs2) to buffer.
void inline AppendAnsiStr(string &buff, string &str) { buff.append(str.c_str(), str.size() + 1); }

// append text included 0x0 and bom(for ucs2) to buffer.
void appendStrByEncodingAndBom(string &buff, cstr_t str, ID3v2EncType encType, CharEncodingType encoding = ED_SYSDEF);

// return the size of bytes used from buffer data
int copyAnsiStr(string &str, const char *data, int nLen);

// return the size of bytes used from buffer data
int copyStrByEncodingAndBom(string &str, ID3v2EncType encType, const char *data, int len, CharEncodingType encoding = ED_SYSDEF);

uint32_t syncBytesToUInt32(uint8_t *bytes);
void syncBytesFromUInt32(uint32_t value, uint8_t *bytesOut);
inline string syncBytesFromUInt32(uint32_t value) {
    uint8_t buf[4];
    syncBytesFromUInt32(value, buf);
    return string((char *)buf, 4);
}

void synchDataDecode(string &data);
string synchDataEncode(const string &data);

uint32_t byteDataToUInt(uint8_t *byData, int nLen);
void byteDataFromUInt(uint32_t value, uint8_t *byData, int nLen);

int fileDataReset(FILE *m_fp, long nOffset, long nSize, int nMask);
int fileDataCmp(FILE *m_fp, long nOffset, const void *lpData, long nSize, int &nResult);
int fileMoveEndData(FILE *m_fp, long nOffset, long nNewOffset);

#endif // !defined(MediaTags_ID3_ID3Helper_h)
