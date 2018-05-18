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

#define WALLET_DATA_MAX_SIZE 10000

namespace Elastos {
	namespace SDK {

		KeyStore::KeyStore() {

		}

		KeyStore::~KeyStore() {

		}

		bool KeyStore::open(const boost::filesystem::path &path, const std::string &password){
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			SjclFile sjclFile;
			from_json(j, sjclFile);

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
			if(!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(),
								  sjclFile.getIter(), dgst, iklen, key)) {
				handleDecryptErrors();
				return false;
			}

			unsigned char plaintext[WALLET_DATA_MAX_SIZE];

			std::vector<unsigned char> ct = SjclBase64::toBits(sjclFile.getCt());
			std::vector<unsigned char> iv = SjclBase64::toBits(sjclFile.getIv());
			std::vector<unsigned char> adata = SjclBase64::toBits(sjclFile.getAdata());
			int ret = decryptccm(ct.data(), 9, adata.data(), adata.size(),
								 &ct.data()[9], key, iv.data(), plaintext);

			nlohmann::json walletJson;
			std::stringstream ss;
			ss << plaintext;
			ss >> walletJson;

			from_json(walletJson, _walletJson);
		}

		bool KeyStore::save(const boost::filesystem::path &path, const std::string &password) {
			return false;
		}

		int KeyStore::decryptccm(unsigned char *ciphertext, int ciphertext_len, unsigned char *aad, int aad_len,
								 unsigned char *tag, unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
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