// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AES_256_CCM_H__
#define __ELASTOS_SDK_AES_256_CCM_H__

#include <cstdint>

#include "CMemBlock.h"

#define CIPHERTEXTMAXLENGTH 1024

namespace Elastos {
	namespace SDK {

		class AES_256_CCM {
			static bool _bInit;
		public:
			static bool Init();

			static CMemBlock<unsigned char>
			encrypt(unsigned char *plaintText, size_t szPlainText, unsigned char *password, size_t szPassword,
					unsigned char *aad = nullptr, size_t szAad = 0);

			static CMemBlock<unsigned char>
			decrypt(unsigned char *cipherText, size_t szCipherText, unsigned char *password, size_t szPassword,
					unsigned char *aad = nullptr, size_t szAad = 0);
		};
	}
}


#endif //__ELASTOS_SDK_AES_256_CCM_H__
