// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KeyStore.h"
#include "SjclFile.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Base64.h>
#include <SDK/Crypto/AES.h>
#include <SDK/Account/StandardAccount.h>
#include <SDK/Account/SimpleAccount.h>
#include <SDK/Account/MultiSignAccount.h>

#include <openssl/evp.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>

#define WALLET_DATA_MAX_SIZE 10000

namespace Elastos {
	namespace ElaWallet {

		KeyStore::KeyStore(const std::string &rootPath) :
			_rootPath(rootPath),
			_walletJson(rootPath) {
		}

		KeyStore::~KeyStore() {
			//EVP_cleanup();
		}

		bool KeyStore::Open(const boost::filesystem::path &path, const std::string &password) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			return Import(j, password);
		}

		bool KeyStore::Import(const nlohmann::json &json, const std::string &passwd, const std::string &phrasePasswd) {
			bool r = Import(json, passwd);

			if (r) {
				_walletJson.SetPhrasePassword(phrasePasswd);
			}

			return r;
		}

		bool KeyStore::Import(const nlohmann::json &json, const std::string &passwd) {
			SjclFile sjcl;
			json >> sjcl;

			if (sjcl.GetMode() != "ccm") {
				ErrorChecker::CheckCondition(true, Error::KeyStore, "Keystore is not ccm mode");
				return false;
			}

			bytes_t plaintext = AES::DecryptCCM(sjcl.GetCt(), passwd, sjcl.GetSalt(), sjcl.GetIv(), sjcl.GetAdata(),
												sjcl.GetKs());

			nlohmann::json walletJson = nlohmann::json::parse(std::string((char *)&plaintext[0], plaintext.size()));
			walletJson >> _walletJson;

			return true;
		}

		bool KeyStore::Save(const boost::filesystem::path &path, const std::string &password) {
			nlohmann::json json;

			Export(json, password);

			std::ofstream outfile(path.string());
			outfile << json;

			return true;
		}

		bool KeyStore::Export(nlohmann::json &json, const std::string &passwd) {

			nlohmann::json plainJson;
			plainJson << _walletJson;

			std::string plaintext = plainJson.dump();

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

			json << sjcl;

			return true;
		}

		const ElaNewWalletJson &KeyStore::json() const {
			return _walletJson;
		}

		ElaNewWalletJson &KeyStore::json() {
			return _walletJson;
		}

		bool KeyStore::IsOld() {
			if (_walletJson.GetCoinInfoList().empty()) {
				return true;
			}
			return false;
		}

		bool KeyStore::HasPhrasePassword() const {
			return _walletJson.HasPhrasePassword();
		}

		IAccount *
		KeyStore::CreateAccountFromJson(const std::string &payPassword) const {
			if (_walletJson.GetType() == "Standard" || _walletJson.GetType().empty())
				return new StandardAccount(_rootPath, _walletJson.GetMnemonic(),
										   _walletJson.GetPhrasePassword(), payPassword);
			else if (_walletJson.GetType() == "Simple")
				return new SimpleAccount(_walletJson.GetPrivateKey(), payPassword);
			else if (_walletJson.GetType() == "MultiSign") {

				if (!_walletJson.GetMnemonic().empty()) {
					return new MultiSignAccount(
						new StandardAccount(_rootPath, _walletJson.GetMnemonic(),
											_walletJson.GetPhrasePassword(), payPassword), _walletJson.GetCoSigners(),
						_walletJson.GetRequiredSignCount());
				} else if (!_walletJson.GetPrivateKey().empty()) {
					return new MultiSignAccount(new SimpleAccount(_walletJson.GetPrivateKey(), payPassword),
												_walletJson.GetCoSigners(), _walletJson.GetRequiredSignCount());
				} else {
					return new MultiSignAccount(nullptr, _walletJson.GetCoSigners(),
												_walletJson.GetRequiredSignCount());
				}
			}

			return nullptr;
		}

		void KeyStore::InitJsonFromAccount(IAccount *account, const std::string &payPassword) {
			ErrorChecker::CheckCondition(account == nullptr, Error::KeyStore, "Account pointer can not be null");
			_walletJson.SetType(account->GetType());

			InitAccountByType(account, payPassword);
		}

		void KeyStore::InitStandardAccount(IAccount *account, const std::string &payPassword) {
			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(account);
			if (standardAccount == nullptr) return;

			bytes_t bytes = AES::DecryptCCM(standardAccount->GetEncryptedMnemonic(), payPassword);
			std::string phrase = std::string((char *)bytes.data(), bytes.size());

			_walletJson.SetMnemonic(phrase);
			_walletJson.SetLanguage(standardAccount->GetLanguage());

			bytes = AES::DecryptCCM(standardAccount->GetEncryptedPhrasePassword(), payPassword);
			std::string phrasePasswd = std::string((char *)bytes.data(), bytes.size());

			_walletJson.SetPhrasePassword(phrasePasswd);
			if (phrasePasswd.empty()) {
				_walletJson.SetHasPhrasePassword(false);
			} else {
				_walletJson.SetHasPhrasePassword(true);
			}
		}

		void KeyStore::InitSimpleAccount(IAccount *account, const std::string &payPassword) {
			SimpleAccount *simpleAccount = dynamic_cast<SimpleAccount *>(account);
			if (simpleAccount == nullptr) return;

			bytes_t prvkey = AES::DecryptCCM(simpleAccount->GetEncryptedKey(), payPassword);
			_walletJson.SetPrivateKey(prvkey.getHex());
		}

		void KeyStore::InitMultiSignAccount(IAccount *account, const std::string &payPassword) {
			MultiSignAccount *multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			if (multiSignAccount == nullptr) return;

			const std::vector<bytes_t> &coSigners = multiSignAccount->GetCoSigners();
			std::vector<std::string> signers;
			for (size_t i = 0; i < coSigners.size(); ++i)
				signers.push_back(coSigners[i].getHex());

			_walletJson.SetCoSigners(signers);
			_walletJson.SetRequiredSignCount(multiSignAccount->GetRequiredSignCount());

			if (multiSignAccount->GetInnerAccount() != nullptr)
				InitAccountByType(multiSignAccount->GetInnerAccount(), payPassword);
		}

		void KeyStore::InitAccountByType(IAccount *account, const std::string &payPassword) {

			if (account->GetType() == "MultiSign") {
				InitMultiSignAccount(account, payPassword);
			} else if (account->GetType() == "Simple") {
				InitSimpleAccount(account, payPassword);
			} else if (account->GetType() == "Standard") {
				InitStandardAccount(account, payPassword);
			}
		}
	}
}