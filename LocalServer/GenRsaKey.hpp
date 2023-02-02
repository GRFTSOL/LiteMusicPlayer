//
//  GenRsaKey.hpp
//  Mp3Player
//
//  Created by henry_xiao on 2023/1/20.
//

#ifndef GenRsaKey_hpp
#define GenRsaKey_hpp

#include "mbedtls/build_info.h"
#include "mbedtls/platform.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include <string>

std::string getMbedtlsError(int err);
int generateRSAKey(mbedtls_pk_context &rsaKey, std::string &privateKey, std::string &publicKey, int keySize = 2048);

#endif /* GenRsaKey_hpp */
