// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_AES_H__
#define __ELASTOS_SDK_AES_H__

#include <Common/typedefs.h>
#include <openssl/ossl_typ.h>

namespace Elastos {
	namespace ElaWallet {

#define AES_DEFAULT_ITER 10000
#define AES_DEFAULT_KS   128

		class AES {
		public:
			static bytes_t RandomSalt();

			static bytes_t RandomIV();


			static std::string EncryptCCM(const bytes_t &plaintext, const std::string &passwd);

			static std::string EncryptCCM(const bytes_t &plaintext, const std::string &passwd, const std::string &salt,
										  const std::string &iv, const std::string &aad = "", int ks = AES_DEFAULT_KS,
										  int iter = AES_DEFAULT_ITER);

			static bytes_t EncryptCCM(const bytes_t &plaintext, const std::string &passwd, const bytes_t &salt,
									  const bytes_t &iv, const bytes_t &aad = bytes_t(), int ks = AES_DEFAULT_KS,
									  int iter = AES_DEFAULT_ITER);


			static bytes_t DecryptCCM(const std::string &ciphertext, const std::string &passwd);

			static bytes_t DecryptCCM(const std::string &ciphertext, const std::string &passwd, const std::string &salt,
									  const std::string &iv, const std::string &aad = "",
									  int ks = AES_DEFAULT_KS, int iter = AES_DEFAULT_ITER);

			static bytes_t DecryptCCM(const bytes_t &ciphertext, const std::string &passwd, const bytes_t &salt,
									  const bytes_t &iv, const bytes_t &aad = bytes_t(),
									  int ks = AES_DEFAULT_KS, int iter = AES_DEFAULT_ITER);


		private:
			static EVP_CIPHER_CTX *Init(bytes_t &key, const std::string &passwd, const bytes_t &salt, int iter);

			static bool EncryptCCM(bytes_t &ciphertext, EVP_CIPHER_CTX *ctx, const bytes_t &plaintext,
								   const bytes_t &aad, const bytes_t &key, const bytes_t &iv, int ks);

			static bool DecryptCCM(bytes_t &plaintext, EVP_CIPHER_CTX *ctx, const bytes_t &ciphertext,
								   const bytes_t &aad, const bytes_t &key, const bytes_t &iv, int ks);

		};
	}
}

#endif //__ELASTOS_SDK_AES_H__

