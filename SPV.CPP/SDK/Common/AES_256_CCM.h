// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AES_256_CCM_H__
#define __ELASTOS_SDK_AES_256_CCM_H__

#include <cstdint>

#include "CMemBlock.h"

#define CIPHERTEXTMAXLENGTH 1024 * 3

namespace Elastos {
	namespace ElaWallet {

		class AES_256_CCM {
			static bool _bInit;
		public:
			static bool Init();

			static bool GenerateSaltAndIV(CMemBlock<unsigned char> &salt, CMemBlock<unsigned char> &iv);

			static CMBlock
			encrypt(unsigned char *plainText, size_t szPlainText, unsigned char *password, size_t szPassword,
					unsigned char *salt, size_t szSalt, unsigned char *iv, size_t szIv, bool bAes128 = false,
					unsigned char *aad = nullptr, size_t szAad = 0);

			static CMBlock
			decrypt(unsigned char *cipherText, size_t szCipherText, unsigned char *password, size_t szPassword,
					unsigned char *salt, size_t szSalt, unsigned char *iv, size_t szIv, bool bAes128 = false,
					unsigned char *aad = nullptr, size_t szAad = 0);
		};
	}
}


#endif //__ELASTOS_SDK_AES_256_CCM_H__
