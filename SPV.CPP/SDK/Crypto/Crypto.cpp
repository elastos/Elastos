// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Crypto.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <openssl/evp.h>
#include <openssl/ssl.h>

static int PLAINTEXTMAXLENGTH = CIPHERTEXTMAXLENGTH;

namespace Elastos {
	namespace ElaWallet {

		bool Crypto::EncryptCCM(CMBlock &ct, CMBlock &tag, const CMBlock &plainText, const CMBlock &adata,
									 const CMBlock &key, const CMBlock &iv, bool AES128) {
			EVP_CIPHER_CTX *ctx;

			int len;
			int ciphertext_len;

			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) {
				Log::error("Encrypt: Create and initialise the context");
				return false;
			}

			/* Initialise the encryption operation. */
			if (1 != EVP_EncryptInit_ex(ctx, AES128 ? EVP_aes_128_ccm() : EVP_aes_256_ccm(), NULL, NULL, NULL)) {
				Log::error("Encrypt: Initialise the encryption operation");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			int ivLen = 2;
			if (plainText.GetSize() >= (1 << 16)) ivLen++;
			if (plainText.GetSize() >= (1 << 24)) ivLen++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - ivLen, NULL)) {
				Log::error("Encrypt: Set IV len to {}", 15 - ivLen);
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Set tag length */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, NULL)) {
				Log::error("Encrypt: Set tag length");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Initialise key and IV */
			if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) {
				Log::error("Encrypt: Initialise key and IV");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Provide the total plaintext length
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, NULL, plainText.GetSize())) {
				Log::error("Encrypt: Provide the total plaintext length");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Provide any AAD data. This can be called zero or one times as
			 * required
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, adata.GetSize() == 0 ? (const unsigned char *) "" : adata, adata.GetSize())) {
				Log::error("Encrypt: Provide any AAD data");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			ct.Resize(CIPHERTEXTMAXLENGTH);
			/* Provide the message to be encrypted, and obtain the encrypted output.
			 * EVP_EncryptUpdate can only be called once for this
			 */
			if (1 != EVP_EncryptUpdate(ctx, ct, &len, plainText, plainText.GetSize())) {
				Log::error("Encrypt: Provide the message to be encrypted");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}
			ciphertext_len = len;

			/* Finalise the encryption. Normally ciphertext bytes may be written at
			 * this stage, but this does not occur in CCM mode
			 */
			if (1 != EVP_EncryptFinal_ex(ctx, &ct[len], &len)) {
				Log::error("Encrypt: Finalise the encryption");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}
			ciphertext_len += len;

			ct.Resize(ciphertext_len);

			tag.Resize(8);
			/* Get the tag */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, 8, tag)) {
				Log::error("Encrypt: Get the tag");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			return true;
		}

		bool Crypto::DecryptCCM(CMBlock &plainText, const CMBlock &ct, const CMBlock &tag, const CMBlock &adata,
									const CMBlock &key, const CMBlock &iv, bool AES128) {
			EVP_CIPHER_CTX *ctx;
			int len;
			plainText.Resize(ct.GetSize());

			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) {
				Log::error("Decrypt: create and initialise the context");
				return false;
			}

			/* Initialise the decryption operation. */
			if (1 != EVP_DecryptInit_ex(ctx, AES128 ? EVP_aes_128_ccm() : EVP_aes_256_ccm(), NULL, NULL, NULL)) {
				Log::error("Decrypt: initialise the decryption operation");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			int ivLen = 2;
			if (ct.GetSize() >= 1 << 16) ivLen++;
			if (ct.GetSize() >= 1 << 24) ivLen++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - ivLen, NULL)) {
				Log::error("Decrypt Setting IV len to {}", 15 - ivLen);
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Set tag length */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, tag.GetSize(), tag)) {
				Log::error("Decrypt: set tag length");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Initialise key and IV */
			if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
				Log::error("Decrypt: initialise key and IV");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Provide the total ciphertext length
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, NULL, ct.GetSize())) {
				Log::error("Decrypt: provide the total ciphertext length");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Provide any AAD data. This can be called zero or more times as
			 * required
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, adata.GetSize() == 0 ? (const unsigned char *)"" : adata, adata.GetSize())) {
				Log::error("Decrypt: provide any AAD data");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			/* Provide the message to be decrypted, and obtain the plaintext output.
			 * EVP_DecryptUpdate can be called multiple times if necessary
			 */
			if (1 != EVP_DecryptUpdate(ctx, plainText, &len, ct, ct.GetSize())) {
				Log::error("Decrypt: update plain text length");
				EVP_CIPHER_CTX_free(ctx);
				return false;
			}

			plainText.Resize(len);

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			return true;
		}

		bool Crypto::_initialized = false;

		void Crypto::Init() {
			if (false == _initialized) {
#if OPENSSL_API_COMPAT < 0x10100000L
				OpenSSL_add_all_algorithms();
#else
				OPENSSL_init_ssl(0, NULL);
#endif
				_initialized = true;
			}
		}

		void Crypto::GenerateSaltAndIV(std::string &saltBase64, std::string &ivBase64) {
			CMBlock salt(8), iv(16);

			for (size_t i = 0; i < 16; i++) {
				if (i < 8) {
					salt[i] = Utils::getRandomByte();
				}
				iv[i] = Utils::getRandomByte();
			}
			saltBase64 = Base64Encode(salt);
			ivBase64 = Base64Encode(iv);
		}

		std::string Crypto::Base64Encode(const CMBlock &buffer) {
			BIO *bio, *b64;
			BUF_MEM *bufferPtr;
			b64 = BIO_new(BIO_f_base64());
			bio = BIO_new(BIO_s_mem());
			bio = BIO_push(b64, bio);

			BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
			BIO_write(bio, buffer, buffer.GetSize());
			BIO_flush(bio);
			BIO_get_mem_ptr(bio, &bufferPtr);
			std::string result((*bufferPtr).data, (*bufferPtr).length);
			BIO_set_close(bio, BIO_CLOSE);
			BIO_free_all(bio);
			return result;
		}

		bool Crypto::Encrypt(std::string &ctBase64, const std::string &plainText, const std::string &passwd,
								  const std::string &saltBase64, const std::string &ivBase64,
								  const std::string &adataBase64, bool AES128) {
			CMBlock	plainData;
			plainData.SetMemFixed((const uint8_t *)plainText.c_str(), plainText.size());
			return Encrypt(ctBase64, plainData, passwd, saltBase64, ivBase64, adataBase64, AES128);
		}

		bool Crypto::Encrypt(std::string &ctBase64, const CMBlock &plainText, const std::string &passwd,
								  const std::string &saltBase64, const std::string &ivBase64,
								  const std::string &adataBase64, bool AES128) {
			CMBlock salt = Base64Decode(saltBase64);
			CMBlock iv = Base64Decode(ivBase64);
			CMBlock adata = Base64Decode(adataBase64);

			Init();
			const EVP_CIPHER *cipher = EVP_get_cipherbyname(LN_aes_256_ccm);
			const EVP_MD *dgst = EVP_sha256();
			if (nullptr != cipher && nullptr != dgst) {
				CMBlock key(EVP_MAX_KEY_LENGTH);
				int keyLen = EVP_CIPHER_key_length(cipher);
				if (PKCS5_PBKDF2_HMAC(passwd.c_str(), passwd.size(), salt, salt.GetSize(), 10000, dgst, keyLen, key)) {
					key.Resize(keyLen);
					CMBlock ct, tag;
					if (!EncryptCCM(ct, tag, plainText, adata, key, iv, AES128)) {
						Log::error("Encrypt: AES CCM");
						return false;
					}

					CMBlock result(ct.GetSize() + tag.GetSize());
					memcpy(result, ct, ct.GetSize());
					memcpy(&result[ct.GetSize()], tag, tag.GetSize());
					ctBase64 = Base64Encode(result);
					return true;
				}
			}

			Log::error("Encrypt error");

			return false;
		}

		size_t Crypto::CalcDecodeLength(const std::string &b64input) {
			size_t len = b64input.size(), padding = 0;
			if (b64input[len - 1] == '=' && b64input[len - 2] == '=') //last two chars are =
				padding = 2;
			else if (b64input[len - 1] == '=') //last char is =
				padding = 1;
			return (len*3)/4 - padding;
		}

		CMBlock Crypto::Base64Decode(const std::string &base64Message) {
			BIO *bio, *b64;
			int decodeLen = CalcDecodeLength(base64Message);

			CMBlock buffer(decodeLen);

			bio = BIO_new_mem_buf(base64Message.c_str(), -1);
			b64 = BIO_new(BIO_f_base64());
			bio = BIO_push(b64, bio);

			BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
			decodeLen = BIO_read(bio, buffer, base64Message.size());
			BIO_free_all(bio);
			buffer.Resize(decodeLen);

			return buffer;
		}

		bool Crypto::Decrypt(std::string &plainText, const std::string &ctBase64, const std::string &passwd,
								  const std::string &saltBase64, const std::string &ivBase64,
								  const std::string &adataBase64, bool AES128) {
			CMBlock plainData;
			if (!Decrypt(plainData, ctBase64, passwd, saltBase64, ivBase64, adataBase64, AES128)) {
				Log::error("Decrypt error");
				return false;
			}

			plainText = std::string(plainData, plainData.GetSize());

			return true;
		}

		bool Crypto::Decrypt(CMBlock &plainText, const std::string &ctBase64, const std::string &passwd,
								  const std::string &saltBase64, const std::string &ivBase64,
								  const std::string &adataBase64, bool AES128) {
			CMBlock ct = Base64Decode(ctBase64);
			CMBlock salt = Base64Decode(saltBase64);
			CMBlock iv = Base64Decode(ivBase64);
			CMBlock adata = Base64Decode(adataBase64);

			if (ct.GetSize() == 0 || 8 > ct.GetSize()) {
				Log::error("ct length {} too short", ct.GetSize());
				return false;
			}

			Init();
			const EVP_CIPHER *cipher = EVP_get_cipherbyname(LN_aes_256_ccm);
			const EVP_MD *dgst = EVP_sha256();
			if (nullptr != cipher && nullptr != dgst) {
				CMBlock key(EVP_MAX_KEY_LENGTH);
				int keyLen = EVP_CIPHER_key_length(cipher);
				if (PKCS5_PBKDF2_HMAC(passwd.c_str(), passwd.size(), salt, salt.GetSize(), 10000, dgst, keyLen, key)) {
					key.Resize(keyLen);
					CMBlock tag(8);
					memcpy(tag, &ct[ct.GetSize() - 8], tag.GetSize());
					ct.Resize(ct.GetSize() - 8);
					if (!DecryptCCM(plainText, ct, tag, adata, key, iv, AES128)) {
						Log::error("Decrypt: AES CCM");
						return false;
					}
					return true;
				}
			}

			Log::error("Decrypt error");

			return false;
		}
	}
}
