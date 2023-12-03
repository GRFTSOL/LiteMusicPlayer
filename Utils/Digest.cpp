#include "Utils.h"
#include "../third-parties/md5/md5.h"


#define MD_CTX              md5_state_t
#define MDInit              md5_init
#define MDUpdate            md5_append
#define MDFinal             md5_finish

void md5ToBinary(const void *input, size_t len, unsigned char digest[]) {
    assert(len >= 0);

    MD_CTX context;

    MDInit (&context);
    MDUpdate (&context, (unsigned char *)input, (int)len);
    MDFinal (&context, digest);
}

string md5ToString(const void *input, size_t len) {
    unsigned char digest[MD5_LEN];

    md5ToBinary((unsigned char *)input, len, digest);
    return hexToStr(digest, MD5_LEN);
}

string md5ToString16(cstr_t szSource) {
    uint8_t digest[16];

    md5ToBinary(szSource, strlen(szSource), digest);
    return hexToStr(digest + 4, 8);
}
