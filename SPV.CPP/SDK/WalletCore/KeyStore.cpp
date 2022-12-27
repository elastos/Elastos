// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KeyStore.h"
#include "SjclFile.h"
#include "AES.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/Base64.h>

#include <openssl/evp.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		KeyStore::KeyStore() {
		}

		KeyStore::KeyStore(const ElaNewWalletJson &walletjson) :
			_walletJson(walletjson) {

		}

		KeyStore::~KeyStore() {
		}

		bool KeyStore::Open(const boost::filesystem::path &path, const std::string &password) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			return Import(j, password);
		}

		bool KeyStore::ImportReadonly(const nlohmann::json &j) {
			_walletJson.FromJson(j);
			return true;
		}

		bool KeyStore::Import(const nlohmann::json &json, const std::string &passwd) {
			SjclFile sjcl;

			sjcl.FromJson(json);

			if (sjcl.GetMode() != "ccm") {
				ErrorChecker::CheckCondition(true, Error::KeyStore, "Keystore is not ccm mode");
				return false;
			}

			bytes_t plaintext = AES::DecryptCCM(sjcl.GetCt(), passwd, sjcl.GetSalt(), sjcl.GetIv(), sjcl.GetAdata(),
												sjcl.GetKs());

			_walletJson.FromJson(nlohmann::json::parse(std::string((char *)&plaintext[0], plaintext.size())));

			return true;
		}

		bool KeyStore::Save(const boost::filesystem::path &path, const std::string &password, bool withPrivKey) {
			nlohmann::json json = Export(password, withPrivKey);

			std::ofstream outfile(path.string());
			outfile << json;

			return true;
		}

		nlohmann::json KeyStore::ExportReadonly() const {
			nlohmann::json roJson = _walletJson.ToJson(false);

			return roJson;
		}

		nlohmann::json KeyStore::Export(const std::string &passwd, bool withPrivKey) const {
			std::string plaintext = _walletJson.ToJson(withPrivKey).dump();

			std::string salt = AES::RandomSalt().getBase64();
			std::string iv = AES::RandomIV().getBase64();
			std::string ciphertext = AES::EncryptCCM(bytes_t(plaintext.c_str(), plaintext.size()), passwd, salt, iv);

			SjclFile sjcl;
			sjcl.SetIv(iv);
			sjcl.SetV(1);
			sjcl.SetIter(AES_DEFAULT_ITER);
			sjcl.SetKs(AES_DEFAULT_KS);
			sjcl.SetTs(64);
			sjcl.SetMode("ccm");
			sjcl.SetAdata("");
			sjcl.SetCipher("aes");
			sjcl.SetSalt(salt);
			sjcl.SetCt(ciphertext);

			return sjcl.ToJson();
		}

	}
}