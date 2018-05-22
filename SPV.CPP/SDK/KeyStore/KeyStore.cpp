// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <openssl/evp.h>

#include "Log.h"
#include "KeyStore.h"
#include "SjclFile.h"
#include "SjclBase64.h"
#include "CMemBlock.h"

#define WALLET_DATA_MAX_SIZE 10000

namespace Elastos {
	namespace SDK {

		KeyStore::KeyStore() {
			OpenSSL_add_all_algorithms();
		}

		KeyStore::~KeyStore() {

		}

		bool KeyStore::open(const boost::filesystem::path &path, const std::string &password) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			SjclFile sjclFile;
			j >> sjclFile;

			if (sjclFile.getMode() != "ccm") {
				handleDecryptErrors();
				return false;
			}
			const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-256-ccm");
			const EVP_MD *dgst = EVP_sha256();

			std::vector<unsigned char> salt = SjclBase64::toBits(sjclFile.getSalt());

			//get key by password and salt
			unsigned char key[EVP_MAX_KEY_LENGTH];
			int iklen = EVP_CIPHER_key_length(cipher);
			if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(),
								   sjclFile.getIter(), dgst, iklen, key)) {
				handleDecryptErrors();
				return false;
			}

			unsigned char plaintext[WALLET_DATA_MAX_SIZE];

			std::vector<unsigned char> ct = SjclBase64::toBits(sjclFile.getCt());
			std::vector<unsigned char> iv = SjclBase64::toBits(sjclFile.getIv());
			std::vector<unsigned char> adata = SjclBase64::toBits(sjclFile.getAdata());
			int ret = decryptccm(ct.data(), ct.size() - 8, nullptr == adata.data() ? "" : adata.data(), adata.size(),
								 &ct.data()[ct.size() - 8], key, iv.data(), plaintext);

			nlohmann::json walletJson;
			std::stringstream ss;
			unsigned char mb[ret];
			memcpy(mb, plaintext, ret);
			ss << mb;
			ss >> walletJson;

			walletJson >> _walletJson;

			return true;
		}

		bool KeyStore::save(const boost::filesystem::path &path, const std::string &password) {
			std::string str_ss;
			nlohmann::json walletJson;
			walletJson << _walletJson;
			std::stringstream ss;
			ss << walletJson;
			ss >> str_ss;

			static const unsigned char iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF,
											   0x21, 0xCA, 0x80, 0xCF};
			const EVP_CIPHER *cipher = EVP_get_cipherbyname("aes-256-ccm");
			const EVP_MD *dgst = EVP_sha256();
			if (nullptr != cipher && nullptr != dgst) {
				static const unsigned char salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};
				unsigned char key[EVP_MAX_KEY_LENGTH];
				int iklen = EVP_CIPHER_key_length(cipher);
				if (PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, 8, 10000, dgst, iklen, key)) {
					unsigned char ciphertext[1000] = {0};
					unsigned char tag[8] = {0};

					int ret = encryptccm(str_ss.c_str(), str_ss.size(), "", 0, key, iv, ciphertext, tag);
					unsigned char ct[ret + 8] = {0};
					memcpy(ct, ciphertext, ret);
					memcpy(ct + ret, tag, 8);
					std::string ct_base64 = SjclBase64::fromBits(ct, sizeof(ct));

					nlohmann::json json;
					json["iv"] = std::string("n2JUTJ0/yrLdCDPfIcqAzw==");
					json["v"] = uint32_t(1);
					json["iter"] = uint32_t(10000);
					json["ks"] = uint32_t(256);
					json["ts"] = uint32_t(64);
					json["mode"] = std::string("ccm");
					json["adata"] = std::string("");
					json["cipher"] = std::string("aes");
					json["salt"] = std::string("ZRVja4LFrFY=");
					json["ct"] = ct_base64;

					std::ofstream outfile(path.string());
					json >> outfile;

					return true;
				}
			}

			return false;
		}

		int KeyStore::encryptccm(unsigned char *plaintext, int plaintext_len, unsigned char *aad,
								 int aad_len, unsigned char *key, unsigned char *iv,
								 unsigned char *ciphertext, unsigned char *tag) {
			EVP_CIPHER_CTX *ctx;

			int len;

			int ciphertext_len;


			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) handleEncryptErrors();

			/* Initialise the encryption operation. */
			if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ccm(), NULL, NULL, NULL))
				handleEncryptErrors();

			int lol = 2;
			if (plaintext_len >= 1 << 16) lol++;
			if (plaintext_len >= 1 << 24) lol++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - lol, NULL))
				handleEncryptErrors();

			/* Set tag length */
			EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, NULL);

			/* Initialise key and IV */
			if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) handleEncryptErrors();

			/* Provide the total plaintext length
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, NULL, plaintext_len))
				handleEncryptErrors();

			/* Provide any AAD data. This can be called zero or one times as
			 * required
			 */
			if (1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
				handleEncryptErrors();

			/* Provide the message to be encrypted, and obtain the encrypted output.
			 * EVP_EncryptUpdate can only be called once for this
			 */
			if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
				handleEncryptErrors();
			ciphertext_len = len;

			/* Finalise the encryption. Normally ciphertext bytes may be written at
			 * this stage, but this does not occur in CCM mode
			 */
			if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleEncryptErrors();
			ciphertext_len += len;

			/* Get the tag */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, 8, tag))
				handleEncryptErrors();

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);

			return ciphertext_len;
		}

		int KeyStore::decryptccm(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad,
								 int aad_len, unsigned char *tag, unsigned char *key, unsigned char *iv,
								 unsigned char *plaintext) {
			EVP_CIPHER_CTX *ctx;
			int len;
			int plaintext_len;
			int ret;

			/* Create and initialise the context */
			if (!(ctx = EVP_CIPHER_CTX_new())) handleDecryptErrors();

			/* Initialise the decryption operation. */
			if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ccm(), NULL, NULL, NULL))
				handleDecryptErrors();

			int lol = 2;
			if (ciphertext_len >= 1 << 16) lol++;
			if (ciphertext_len >= 1 << 24) lol++;

			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, 15 - lol, NULL))
				handleDecryptErrors();

			/* Set expected tag value. */
			if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, 8, tag))
				handleDecryptErrors();

			/* Initialise key and IV */
			if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) handleDecryptErrors();

			/* Provide the total ciphertext length
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, NULL, ciphertext_len))
				handleDecryptErrors();

			/* Provide any AAD data. This can be called zero or more times as
			 * required
			 */
			if (1 != EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
				handleDecryptErrors();

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

		void KeyStore::handleEncryptErrors() {

		}

		void KeyStore::handleDecryptErrors() {
			Log::error("Invalid wallet data.");
		}

		const std::string &KeyStore::getMnemonic() const {
			return _walletJson.getMnemonic();
		}

		const std::string &KeyStore::getMasterPrivateKey() const {
			//todo get from BitcoreWalletClientJson
			return "";
		}

		const std::string &KeyStore::getID() const {
			return _walletJson.getID();
		}

		const std::string &KeyStore::getIDInfo() const {
			return _walletJson.getIDInfo();
		}

		const ElaNewWalletJson &KeyStore::json() const {
			return _walletJson;
		}

		ElaNewWalletJson &KeyStore::json() {
			return _walletJson;
		}
	}
}