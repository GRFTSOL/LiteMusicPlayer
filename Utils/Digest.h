#pragma once

#define MD5_LEN            16
#define MD5_STR_LEN        (32 + 1)

void md5ToBinary(const void *input, size_t len, unsigned char digest[]);

string md5ToString(const void *input, size_t len);

string md5ToString16(cstr_t szSource);

inline void md5ToBinary(cstr_t str, unsigned char digest[]) {
    md5ToBinary(str, (int)strlen(str) * sizeof(char), digest);
}
