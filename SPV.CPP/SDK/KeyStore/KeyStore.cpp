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
				return false;
			}

			std::vector<unsigned char> ct = Base64::toBits(sjclFile.getCt());
			std::vector<unsigned char> adata = Base64::toBits(sjclFile.getAdata());
			CMBlock plaintext = AES_256_CCM::decrypt(ct.data(), ct.size(),
																	  (unsigned char *) password.c_str(),
																	  password.size(), adata.data(), adata.size());
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
			std::string str_ss;
			nlohmann::json walletJson;
			walletJson << _walletJson;
			std::stringstream ss;
			ss << walletJson;
			ss >> str_ss;

			CMBlock ciphertext;
			ciphertext = AES_256_CCM::encrypt((unsigned char *) str_ss.c_str(), str_ss.size(),
											  (unsigned char *) password.c_str(), password.size());
			if (false == ciphertext)
				return false;

			std::string ct_base64 = Base64::fromBits(ciphertext, ciphertext.GetSize());
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