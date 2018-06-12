// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <openssl/evp.h>
#include "Utils.h"

#include "AES_256_CCM.h"


static int PLAINTEXTMAXLENGTH = CIPHERTEXTMAXLENGTH;

namespace Elastos {
	namespace SDK {
		void _handleEncryptErrors() {
		}

		void _handleDecryptErrors() {
		}

		static int _encryptccm(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
							   int aad_len, unsigned char *key, const unsigned char *iv,
							   unsigned char *ciphertext, unsigned char *tag, bool bAes128 = false) {
			EVP_CIPHER_CTX *ctx;

			int len;

			int ciphertext_len;


			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) _handleEncryptErrors();

			/* Initialise the encryption operation. */
			if (1 != EVP_EncryptInit_ex(ctx, !bAes128 ? EVP_aes_256_ccm() : EVP_aes_128_ccm(), NULL, NULL, NULL))
				_handleEncryptErrors();

			int lol = 2;
			if (plaintext_len >= 1 << 16) lol++;
			if (plaintext_len >= 1 << 24) lol++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - lol, NULL))
				_handleEncryptErrors();

			/* Set tag length */
			EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, NULL);

			/* Initialise key and IV */
			if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) _handleEncryptErrors();

			/* Provide the total plaintext length
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, NULL, plaintext_len))
				_handleEncryptErrors();

			/* Provide any AAD data. This can be called zero or one times as
			 * required
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
				_handleEncryptErrors();

			/* Provide the message to be encrypted, and obtain the encrypted output.
			 * EVP_EncryptUpdate can only be called once for this
			 */
			if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
				_handleEncryptErrors();
			ciphertext_len = len;

			/* Finalise the encryption. Normally ciphertext bytes may be written at
			 * this stage, but this does not occur in CCM mode
			 */
			if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) _handleEncryptErrors();
			ciphertext_len += len;

			/* Get the tag */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, 8, tag))
				_handleEncryptErrors();

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			return ciphertext_len;
		}

		static int _decryptccm(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
							   int aad_len, unsigned char *tag, unsigned char *key, const unsigned char *iv,
							   unsigned char *plaintext, bool bAes128 = false) {
			EVP_CIPHER_CTX *ctx;
			int len;
			int plaintext_len;
			int ret;

			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) _handleDecryptErrors();

			/* Initialise the decryption operation. */
			if (1 != EVP_DecryptInit_ex(ctx, !bAes128 ? EVP_aes_256_ccm() : EVP_aes_128_ccm(), NULL, NULL, NULL))
				_handleDecryptErrors();

			int lol = 2;
			if (ciphertext_len >= 1 << 16) lol++;
			if (ciphertext_len >= 1 << 24) lol++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - lol, NULL))
				_handleDecryptErrors();

			/* Set expected tag value. */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, tag))
				_handleDecryptErrors();

			/* Initialise key and IV */
			if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) _handleDecryptErrors();

			/* Provide the total ciphertext length
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, NULL, ciphertext_len))
				_handleDecryptErrors();

			/* Provide any AAD data. This can be called zero or more times as
			 * required
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
				_handleDecryptErrors();

			/* Provide the message to be decrypted, and obtain the plaintext output.
			 * EVP_DecryptUpdate can be called multiple times if necessary
			 */
			ret = EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);

			plaintext_len = len;

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			if (ret > 0) {
				/* Success */
				return plaintext_len;
			} else {
				/* Verify failed */
				return -1;
			}
		}

		bool AES_256_CCM::_bInit = false;

		bool AES_256_CCM::Init() {
			if (false == _bInit) {
				OpenSSL_add_all_algorithms();
				_bInit = true;
			}

			return true;
		}

		bool AES_256_CCM::GenerateSaltAndIV(CMemBlock<unsigned char> &salt, CMemBlock<unsigned char> &iv) {
			static unsigned char _salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};
			static unsigned char _iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF, 0x21,
										  0xCA, 0x80, 0xCF};
			for (size_t i = 0; i < 16; i++) {

				if (i < 8) {
					_salt[i] = Utils::getRandomByte();
				}
				_iv[i] = Utils::getRandomByte();
			}
			salt.SetMemFixed(_salt, sizeof(_salt));
			iv.SetMemFixed(_iv, sizeof(_iv));

			return true;
		}

		CMBlock
		AES_256_CCM::encrypt(unsigned char *plainText, size_t szPlainText, unsigned char *password, size_t szPassword,
							 unsigned char *salt, size_t szSalt, unsigned char *iv, size_t szIv, bool bAes128,
							 unsigned char *aad, size_t szAad) {
			CMBlock _ret;

			Init();
			const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-256-ccm");
			const EVP_MD *dgst = EVP_sha256();
			if (nullptr != cipher && nullptr != dgst) {
				unsigned char key[EVP_MAX_KEY_LENGTH];
				int iklen = EVP_CIPHER_key_length(cipher);
				if (PKCS5_PBKDF2_HMAC((char *) password, szPassword, salt, szSalt, 10000, dgst, iklen, key)) {
					unsigned char ciphertext[CIPHERTEXTMAXLENGTH] = {0};
					static unsigned char tag[8] = {0};
					int ret = 0;
					try {
						ret = _encryptccm(plainText, szPlainText, nullptr == aad ? (unsigned char *) "" : aad, szAad,
										  key, iv, ciphertext, tag, bAes128);
					}
					catch (...) {
						return _ret;
					}
					if (0 < ret) {
						_ret.Resize(ret + 8);
						memcpy(_ret, ciphertext, ret);
						memcpy((unsigned char *) _ret + ret, tag, 8);
					}
				}
			}

			return _ret;
		}

		CMBlock
		AES_256_CCM::decrypt(unsigned char *cipherText, size_t szCipherText, unsigned char *password, size_t szPassword,
							 unsigned char *salt, size_t szSalt, unsigned char *iv, size_t szIv, bool bAes128,
							 unsigned char *aad, size_t szAad) {
			CMBlock _ret;

			if (nullptr == cipherText || 8 >= szCipherText) {
				return _ret;
			}

			Init();
			const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-256-ccm");
			const EVP_MD *dgst = EVP_sha256();
			if (nullptr != cipher && nullptr != dgst) {
				unsigned char key[EVP_MAX_KEY_LENGTH];
				int iklen = EVP_CIPHER_key_length(cipher);
				if (PKCS5_PBKDF2_HMAC((char *) password, szPassword, salt, szSalt, 10000, dgst, iklen, key)) {
					unsigned char plaintext[CIPHERTEXTMAXLENGTH] = {0};
					int ret = 0;
					try {
						ret = _decryptccm(cipherText, szCipherText - 8, nullptr == aad ? (unsigned char *) "" : aad,
										  szAad, &cipherText[szCipherText - 8], key, iv, plaintext, bAes128);
					}
					catch (...) {
						return _ret;
					}
					if (0 < ret) {
						_ret.Resize(ret);
						memcpy(_ret, plaintext, ret);
					}
				}
			}

			return _ret;
		}
	}
}