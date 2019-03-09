// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KeyStore.h"
#include "SjclFile.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/CMemBlock.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Base64.h>
#include <SDK/Crypto/Crypto.h>
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

			CMBlock plainText;
			ErrorChecker::CheckDecrypt(!Crypto::Decrypt(plainText, sjcl.GetCt(), passwd, sjcl.GetSalt(), sjcl.GetIv(),
														sjcl.GetAdata(), sjcl.GetKs() == 128 ? true : false));

			std::string plainString(plainText, plainText.GetSize());
			nlohmann::json walletJson = nlohmann::json::parse(plainString);
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

			std::string ctBase64;

			nlohmann::json plainJson;
			plainJson << _walletJson;

			std::string plainString = plainJson.dump();
			std::string saltBase64, ivBase64;
			Crypto::GenerateSaltAndIV(saltBase64, ivBase64);
			std::string adataBase64 = "";
			bool AES128 = false;
			Crypto::Encrypt(ctBase64, plainString, passwd, saltBase64, ivBase64, adataBase64, AES128);

			SjclFile sjcl;
			sjcl.SetIv(ivBase64);
			sjcl.SetV(1);
			sjcl.SetIter(10000);
			sjcl.SetKs(AES128 ? 128 : 256);
			sjcl.SetTs(64);
			sjcl.SetMode("ccm");
			sjcl.SetAdata(adataBase64);
			sjcl.SetCipher("aes");
			sjcl.SetSalt(saltBase64);
			sjcl.SetCt(ctBase64);

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

			std::string phrase;
			ErrorChecker::CheckDecrypt(!Utils::Decrypt(phrase, standardAccount->GetEncryptedMnemonic(), payPassword));
			_walletJson.SetMnemonic(phrase);
			_walletJson.SetLanguage(standardAccount->GetLanguage());
			std::string phrasePasswd;
			ErrorChecker::CheckDecrypt(!Utils::Decrypt(phrasePasswd, standardAccount->GetEncryptedPhrasePassword(),
													   payPassword));
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

			CMBlock privKey;
			ErrorChecker::CheckDecrypt(!Utils::Decrypt(privKey, simpleAccount->GetEncryptedKey(), payPassword));
			_walletJson.SetPrivateKey(Utils::EncodeHex(privKey));
		}

		void KeyStore::InitMultiSignAccount(IAccount *account, const std::string &payPassword) {
			MultiSignAccount *multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			if (multiSignAccount == nullptr) return;

			_walletJson.SetCoSigners(multiSignAccount->GetCoSigners());
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