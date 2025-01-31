//
//  GenRsaKey.cpp
//  MusicPlayer
//
//  Created by henry_xiao on 2023/1/20.
//

#include "GenRsaKey.hpp"


static int getPublicPrivateKey(mbedtls_pk_context *key, std::string &privateKey, std::string &publicKey) {
    unsigned char output_buf[16000];

    memset(output_buf, 0, 16000);
    int ret = mbedtls_pk_write_key_pem(key, output_buf, sizeof(output_buf));
    if (ret != 0) {
        return (ret);
    }
    privateKey.assign((char *)output_buf);

    memset(output_buf, 0, 16000);
    ret = mbedtls_pk_write_pubkey_pem(key, output_buf, sizeof(output_buf));
    if (ret != 0) {
        return (ret);
    }
    publicKey.assign((char *)output_buf);

    return (0);
}

std::string getMbedtlsError(int err) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));

    mbedtls_strerror(err, buf, sizeof(buf));
    return buf;
}

int generateRSAKey(mbedtls_pk_context &rsaKey, std::string &privateKey, std::string &publicKey, int keySize) {
    mbedtls_ctr_drbg_context ctr_drbg;
    const char *pers = "gen_key";

    mbedtls_pk_init(&rsaKey);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);

    int ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
        (const unsigned char *)pers, strlen(pers));
    if (ret != 0) {
        goto exit;
    }


    // Generate the key
    ret = mbedtls_pk_setup(&rsaKey, mbedtls_pk_info_from_type((mbedtls_pk_type_t)MBEDTLS_PK_RSA));
    if (ret != 0) {
        goto exit;
    }

    ret = mbedtls_rsa_gen_key(mbedtls_pk_rsa(rsaKey), mbedtls_ctr_drbg_random, &ctr_drbg, keySize, 65537);
    if (ret != 0) {
        goto exit;
    }

    // Export key
    ret = ret = getPublicPrivateKey(&rsaKey, privateKey, publicKey);
    if (ret != 0) {
        goto exit;
    }

exit:
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}
