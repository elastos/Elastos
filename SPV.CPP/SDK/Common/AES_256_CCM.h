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

			static void GenerateSaltAndIV(std::string &saltBase64, std::string &ivBase64);

			static bool Encrypt(std::string &ctBase64, const std::string &plainText, const std::string &passwd,
								const std::string &saltBase64, const std::string &ivBase64,
								const std::string &adataBase64, bool AES128 = false);

			static bool Encrypt(std::string &ctBase64, const CMBlock &plainText, const std::string &passwd,
								const std::string &saltBase64, const std::string &ivBase64,
								const std::string &adataBase64, bool AES128 = false);


			static bool Decrypt(std::string &plainText, const std::string &ctBase64, const std::string &passwd,
								const std::string &saltBase64, const std::string &ivBase64,
								const std::string &adataBase64, bool AES128 = false);

			static bool Decrypt(CMBlock &plainText, const std::string &ctBase64, const std::string &passwd,
								const std::string &saltBase64, const std::string &ivBase64,
								const std::string &adataBase64, bool AES128 = false);

//		private:
			static std::string Base64Encode(const CMBlock &buffer);

			static size_t CalcDecodeLength(const std::string &b64input);
			static CMBlock Base64Decode(const std::string &base64Message);

			static bool EncryptCCM(CMBlock &ct, CMBlock &tag, const CMBlock &plainText, const CMBlock &adata,
								   const CMBlock &key, const CMBlock &iv, bool AES128);
			static bool DecryptCCM(CMBlock &plainText, const CMBlock &ct, const CMBlock &tag, const CMBlock &adata,
								   const CMBlock &key, const CMBlock &iv, bool AES128);
		};
	}
}


#endif //__ELASTOS_SDK_AES_256_CCM_H__
