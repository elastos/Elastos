// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AES.h"
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Common/typedefs.h>
#include <Common/Utils.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace Elastos {
	namespace ElaWallet {



		bytes_t AES::RandomSalt() {
			bytes_t salt;

			for (int i = 0; i < 8; ++i)
				salt.push_back(Utils::getRandomByte());

			return salt;
		}

		bytes_t AES::RandomIV() {
			bytes_t iv;

			for (int i = 0; i < 16; ++i)
				iv.push_back(Utils::getRandomByte());

			return iv;
		}

		std::string AES::EncryptCCM(const bytes_t &plaintext, const std::string &passwd) {
			std::string salt = "ZRVja4LFrFY=";
			std::string iv = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return EncryptCCM(plaintext, passwd, salt, iv);
		}

		std::string AES::EncryptCCM(const bytes_t &plaintext, const std::string &passwd, const std::string &salt,
									const std::string &iv, const std::string &aad, int ks, int iter) {
			bytes_t saltBytes, ivBytes, aadBytes;

			saltBytes.setBase64(salt);
			ivBytes.setBase64(iv);
			aadBytes.setBase64(aad);

			return EncryptCCM(plaintext, passwd, saltBytes, ivBytes, aadBytes, ks, iter).getBase64();
		}

		bytes_t AES::EncryptCCM(const bytes_t &plaintext, const std::string &passwd, const bytes_t &salt,
								const bytes_t &iv, const bytes_t &aad, int ks, int iter) {
			bytes_t ciphertext;
			if (plaintext.empty()) {
				return ciphertext;
			}

			bytes_t key;
			EVP_CIPHER_CTX *ctx = Init(key, passwd, salt, iter);
			if (!ctx) {
				Log::error("aes encrypt init error");
				return bytes_t();
			}

			bool result = EncryptCCM(ciphertext, ctx, plaintext, aad, key, iv, ks);

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			if (!result) {
				ErrorChecker::ThrowLogicException(Error::EncryptError, "encrypt error");
				return bytes_t();
			}

			return ciphertext;
		}

		bool AES::EncryptCCM(bytes_t &ciphertext, EVP_CIPHER_CTX *ctx, const bytes_t &plaintext, const bytes_t &aad,
							 const bytes_t &key, const bytes_t &iv, int ks) {

			const EVP_CIPHER *cipher = nullptr;
			int len;
			bytes_t tag(8);

			if (ks == 128) {
				cipher = EVP_aes_128_ccm();
			} else if (ks == 192) {
				cipher = EVP_aes_192_ccm();
			} else if (ks == 256) {
				cipher = EVP_aes_256_ccm();
			}

			if (!cipher) {
				Log::error("encrypt get cipher fail with ks = {}", ks);
				return false;
			}

			/* Initialise the encryption operation. */
			if (1 != EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL)) {
				Log::error("encrypt init cipher");
				return false;
			}

			int ivLen = 13;
			if (plaintext.size() >= (1 << 16)) ivLen--;
			if (plaintext.size() >= (1 << 24)) ivLen--;

			/* Set IV length. Not necessary if this is 12 bytes (96 bits) */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, ivLen, NULL)) {
				Log::error("encrypt set IV len to {}", ivLen);
				return false;
			}

			/* Set tag length */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, NULL)) {
				Log::error("encrypt set tag length");
				return false;
			}

			/* Initialise key and IV */
			if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data())) {
				Log::error("encrypt initialise key and iv");
				return false;
			}

			ciphertext.resize(plaintext.size());
			/* Provide the total plaintext length */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, NULL, (int)plaintext.size())) {
				Log::error("encrypt provide the total plaintext length");
				return false;
			}

			/* Provide any AAD data. This can be called zero or one times as required */
			if (!aad.empty() && 1 != EVP_EncryptUpdate(ctx, NULL, &len, &aad[0], (int)aad.size())) {
				Log::error("encrypt provide any AAD data");
				return false;
			}

			/* Provide the message to be encrypted, and obtain the encrypted output.
			 * EVP_EncryptUpdate can only be called once for this
			 */
			if (1 != EVP_EncryptUpdate(ctx, &ciphertext[0], &len, &plaintext[0], plaintext.size())) {
				Log::error("encrypt provide the message to be encrypted");
				return false;
			}
			assert(len == ciphertext.size());

			/* Finalise the encryption. Normally ciphertext bytes may be written at
			 * this stage, but this does not occur in CCM mode
			 */
			if (1 != EVP_EncryptFinal_ex(ctx, nullptr, &len)) {
				Log::error("encrypt finalise the encryption");
				return false;
			}

			/* Get the tag */
			int r = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, 8, &tag[0]);
			if (1 != r) {
				Log::error("encrypt get the tag");
				return false;
			}

			ciphertext += tag;

			return true;
		}

		bytes_t AES::DecryptCCM(const std::string &ciphertext, const std::string &passwd) {
			std::string salt = "ZRVja4LFrFY=";
			std::string iv = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return DecryptCCM(ciphertext, passwd, salt, iv);
		}

		bytes_t AES::DecryptCCM(const std::string &ciphertext, const std::string &passwd, const std::string &salt,
								const std::string &iv, const std::string &aad, int ks, int iter) {
			bytes_t ct, saltBytes, ivBytes, aadBytes;

			ct.setBase64(ciphertext);
			saltBytes.setBase64(salt);
			ivBytes.setBase64(iv);
			aadBytes.setBase64(aad);

			return DecryptCCM(ct, passwd, saltBytes, ivBytes, aadBytes, ks, iter);
		}

		bytes_t AES::DecryptCCM(const bytes_t &ciphertext, const std::string &passwd, const bytes_t &salt,
								const bytes_t &iv, const bytes_t &aad, int ks, int iter) {
			bytes_t plaintext;
			if (ciphertext.empty()) {
				return plaintext;
			}

			bytes_t key;
			EVP_CIPHER_CTX *ctx = Init(key, passwd, salt, iter);
			if (!ctx) {
				Log::error("aes encrypt init error");
				return bytes_t();
			}

			bool result = DecryptCCM(plaintext, ctx, ciphertext, aad, key, iv, ks);

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			if (!result) {
				ErrorChecker::ThrowLogicException(Error::WrongPasswd, "Wrong passwd");
				return bytes_t();
			}

			return plaintext;
		}

		bool AES::DecryptCCM(bytes_t &plaintext, EVP_CIPHER_CTX *ctx, const bytes_t &ciphertext,
							 const bytes_t &aad, const bytes_t &key, const bytes_t &iv, int ks) {
			const EVP_CIPHER *cipher = nullptr;
			if (ks == 128) {
				cipher = EVP_aes_128_ccm();
			} else if (ks == 192) {
				cipher = EVP_aes_192_ccm();
			} else if (ks == 256) {
				cipher = EVP_aes_256_ccm();
			}

			if (!cipher) {
				Log::error("decrypt get cipher fail with ks = {}", ks);
				return false;
			}

			if (ciphertext.size() < 8) {
				Log::error("decrypt cipher text len={} too short", ciphertext.size());
				return false;
			}

			bytes_t tag;
			tag.assign(ciphertext.begin() + ciphertext.size() - 8, ciphertext.end());

			/* Initialise the decryption operation. */
			if (1 != EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL)) {
				Log::error("decrypt initialise the decryption operation");
				return false;
			}

			int ivLen = 13;
			if (ciphertext.size() - tag.size() >= (1 << 16)) ivLen--;
			if (ciphertext.size() - tag.size() >= (1 << 24)) ivLen--;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, ivLen, NULL)) {
				Log::error("decrypt setting iv len to {}", ivLen);
				return false;
			}

			/* Set tag length */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, tag.size(), &tag[0])) {
				Log::error("decrypt set tag length");
				return false;
			}

			/* Initialise key and IV */
			if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, &key[0], &iv[0])) {
				Log::error("decrypt initialise key and iv");
				return false;
			}

			int len;
			/* Provide the total ciphertext length */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, NULL, ciphertext.size() - tag.size())) {
				Log::error("decrypt provide the total ciphertext length");
				return false;
			}

			/* Provide any AAD data. This can be called zero or more times as required */
			if (!aad.empty() &&  1 != EVP_DecryptUpdate(ctx, NULL, &len, &aad[0], aad.size())) {
				Log::error("decrypt provide any AAD data");
				return false;
			}

			plaintext.resize(ciphertext.size() - tag.size());
			/* Provide the message to be decrypted, and obtain the plaintext output.
			 * EVP_DecryptUpdate can be called multiple times if necessary
			 */
			if (1 != EVP_DecryptUpdate(ctx, &plaintext[0], &len, &ciphertext[0], ciphertext.size() - tag.size())) {
				Log::error("decrypt update plain text length");
				return false;
			}

			assert(len == plaintext.size());

			return true;
		}

		EVP_CIPHER_CTX *AES::Init(bytes_t &key, const std::string &passwd, const bytes_t &salt, int iter) {
			int keyLen = EVP_CIPHER_key_length(EVP_aes_256_ccm());
			key.resize(keyLen);
			if (!PKCS5_PBKDF2_HMAC(passwd.c_str(), passwd.size(), salt.data(), salt.size(), iter, EVP_sha256(), keyLen, key.data())) {
				return nullptr;
			}

			return EVP_CIPHER_CTX_new();
		}

	}
}
