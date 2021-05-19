////////////////////////////////////////////////////////////////////////////////
//
// hash.h
//
// Copyright (c) 2011-2012 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef __HASH_H___
#define __HASH_H___

#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/hmac.h>

#include <Common/uchar_vector.h>

//#include "hashblock.h" // for Hash9
//#include "scrypt/scrypt.h" // for scrypt_1024_1_1_256

// All inputs and outputs are big endian

inline uchar_vector sha256(const uchar_vector& data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, &data[0], data.size());
    SHA256_Final(hash, &sha256);
    uchar_vector rval(hash, SHA256_DIGEST_LENGTH);
    return rval;
}

inline uchar_vector sha256_2(const uchar_vector& data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, &data[0], data.size());
    SHA256_Final(hash, &sha256);
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, hash, SHA256_DIGEST_LENGTH);
    SHA256_Final(hash, &sha256);
    uchar_vector rval(hash, SHA256_DIGEST_LENGTH);
    return rval;
}

inline uchar_vector ripemd160(const uchar_vector& data)
{
    unsigned char hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160_CTX ripemd160;
    RIPEMD160_Init(&ripemd160);
    RIPEMD160_Update(&ripemd160, &data[0], data.size());
    RIPEMD160_Final(hash, &ripemd160);
    uchar_vector rval(hash, RIPEMD160_DIGEST_LENGTH);
    return rval;
}

inline uchar_vector hash160(const uchar_vector& data)
{
    return ripemd160(sha256(data));
}

inline uchar_vector mdsha(const uchar_vector& data)
{
    return ripemd160(sha256(data));
}

inline uchar_vector sha1(const uchar_vector& data)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, &data[0], data.size());
    SHA1_Final(hash, &sha1);
    uchar_vector rval(hash, SHA_DIGEST_LENGTH);
    return rval;
}

inline uchar_vector hmac_sha256(const uchar_vector& key, const uchar_vector& data)
{
    unsigned char* digest = HMAC(EVP_sha256(), (unsigned char*)&key[0], key.size(), (unsigned char*)&data[0], data.size(), NULL, NULL);
    return uchar_vector(digest, 32);
}

inline uchar_vector hmac_sha512(const uchar_vector& key, const uchar_vector& data)
{
    unsigned char* digest = HMAC(EVP_sha512(), (unsigned char*)&key[0], key.size(), (unsigned char*)&data[0], data.size(), NULL, NULL);
    return uchar_vector(digest, 64);
}

//inline uchar_vector hash9(const uchar_vector& data)
//{
//    uint256 hash = Hash9((unsigned char*)&data[0], (unsigned char*)&data[0] + data.size());
//    return uchar_vector((unsigned char*)&hash, (unsigned char*)&hash + 32);
//}

//inline uchar_vector sha3_256(const uchar_vector& data)
//{
//    uchar_vector hash(32);
//    sph_keccak256_context ctx_keccak;
//    sph_keccak256_init(&ctx_keccak);
//    sph_keccak256(&ctx_keccak, (unsigned char*)&data[0], data.size());
//    sph_keccak256_close(&ctx_keccak, (unsigned char*)&hash[0]);
//    return hash;
//}

//inline uchar_vector scrypt_1024_1_1_256(const uchar_vector& data)
//{
//    uint256 hash;
//    scrypt_1024_1_1_256_((const char*)&data[0], (char*)&hash);
//    return uchar_vector((unsigned char*)&hash, (unsigned char*)&hash + 32);
//}

#endif
