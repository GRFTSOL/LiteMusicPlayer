#pragma once

#ifndef Utils_old_UUEncode_h
#define Utils_old_UUEncode_h


void encodeuu(const char *strIn, int nLen, string &str, bool bBase64 = true, bool bIncNewLine = false);

bool decodebase64(cstr_t szIn, int nLen, string &strOut);

bool decodestduu(cstr_t szIn, int nLen, string &strOut);


#endif // !defined(Utils_old_UUEncode_h)
