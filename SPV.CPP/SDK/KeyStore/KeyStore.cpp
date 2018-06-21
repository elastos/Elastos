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
#include "Base64.h"
#include "CMemBlock.h"
#include "AES_256_CCM.h"

#define WALLET_DATA_MAX_SIZE 10000

namespace Elastos {
	namespace ElaWallet {

		KeyStore::KeyStore() {
			OpenSSL_add_all_algorithms();
		}

		KeyStore::~KeyStore() {
			//EVP_cleanup();
		}

		bool KeyStore::open(const boost::filesystem::path &path, const std::string &password) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			return open(j, password);
		}

		bool KeyStore::open(const nlohmann::json &json, const std::string &password) {
			SjclFile sjclFile;
			json >> sjclFile;

			if (sjclFile.getMode() != "ccm") {
				return false;
			}

			std::vector<unsigned char> ct = Base64::toBits(sjclFile.getCt());
			std::vector<unsigned char> salt = Base64::toBits(sjclFile.getSalt());
			std::vector<unsigned char> iv = Base64::toBits(sjclFile.getIv());
			std::vector<unsigned char> adata = Base64::toBits(sjclFile.getAdata());
			uint32_t ks = sjclFile.getKs();
			CMBlock plaintext;
			plaintext = AES_256_CCM::decrypt(ct.data(), ct.size(), (unsigned char *) password.c_str(), password.size(),
											 salt.data(), salt.size(), iv.data(), iv.size(), 128 == ks ? true : false,
											 adata.data(), adata.size());
			if (false == plaintext)
				return false;

			nlohmann::json walletJson;
			std::stringstream ss;
			unsigned char mb[plaintext.GetSize()];
			memcpy(mb, plaintext, plaintext.GetSize());
			ss << mb;
			ss >> walletJson;

			walletJson >> _walletJson;

			return true;
		}

		bool KeyStore::save(const boost::filesystem::path &path, const std::string &password) {
			nlohmann::json json;
			save(json, password);

			std::ofstream outfile(path.string());
			json >> outfile;

			return true;
		}

		bool KeyStore::save(nlohmann::json &json, const std::string &password) {
			std::string str_ss;
			nlohmann::json walletJson;
			walletJson << _walletJson;
			std::stringstream ss;
			ss << walletJson;
			str_ss = ss.str();

			CMemBlock<unsigned char> salt, iv;
			AES_256_CCM::GenerateSaltAndIV(salt, iv);
			CMBlock ciphertext;
			bool bAes128 = false;
			ciphertext = AES_256_CCM::encrypt((unsigned char *) str_ss.c_str(), str_ss.size(),
											  (unsigned char *) password.c_str(), password.size(), salt, salt.GetSize(),
											  iv, iv.GetSize(), bAes128);
			if (false == ciphertext)
				return false;
			std::string salt_base64 = Base64::fromBits(salt, salt.GetSize());
			std::string iv_base64 = Base64::fromBits(iv, iv.GetSize());
			std::string ct_base64 = Base64::fromBits(ciphertext, ciphertext.GetSize());

			SjclFile sjclFile;
			sjclFile.setIv(iv_base64);
			sjclFile.setV(1);
			sjclFile.setIter(10000);
			sjclFile.setKs(!bAes128 ? uint32_t(256) : uint32_t(128));
			sjclFile.setTs(64);
			sjclFile.setMode("ccm");
			sjclFile.setAdata("");
			sjclFile.setCipher("aes");
			sjclFile.setSalt(salt_base64);
			sjclFile.setCt(ct_base64);
			json << sjclFile;
			return true;
		}

		const ElaNewWalletJson &KeyStore::json() const {
			return _walletJson;
		}

		ElaNewWalletJson &KeyStore::json() {
			return _walletJson;
		}
	}
}