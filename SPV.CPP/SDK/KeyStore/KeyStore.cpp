// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "KeyStore.h"
#include "SjclFile.h"

#include <SDK/Common/ParamChecker.h>
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

		bool KeyStore::open(const boost::filesystem::path &path, const std::string &password) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			return Import(j, password);
		}

		bool KeyStore::Import(const nlohmann::json &json, const std::string &passwd, const std::string &phrasePasswd) {
			bool r = Import(json, passwd);

			if (r) {
				_walletJson.setPhrasePassword(phrasePasswd);
			}

			return r;
		}

		bool KeyStore::Import(const nlohmann::json &json, const std::string &passwd) {
			SjclFile sjcl;
			json >> sjcl;

			if (sjcl.getMode() != "ccm") {
				ParamChecker::checkCondition(true, Error::KeyStore, "Keystore is not ccm mode");
				return false;
			}

			CMBlock plainText;
			ParamChecker::CheckDecrypt(!Crypto::Decrypt(plainText, sjcl.getCt(), passwd, sjcl.getSalt(), sjcl.getIv(),
															 sjcl.getAdata(), sjcl.getKs() == 128 ? true : false));

			std::string plainString(plainText, plainText.GetSize());
			nlohmann::json walletJson = nlohmann::json::parse(plainString);
			walletJson >> _walletJson;

			return true;
		}

		bool KeyStore::save(const boost::filesystem::path &path, const std::string &password) {
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
			sjcl.setIv(ivBase64);
			sjcl.setV(1);
			sjcl.setIter(10000);
			sjcl.setKs(AES128 ? 128 : 256);
			sjcl.setTs(64);
			sjcl.setMode("ccm");
			sjcl.setAdata(adataBase64);
			sjcl.setCipher("aes");
			sjcl.setSalt(saltBase64);
			sjcl.setCt(ctBase64);

			json << sjcl;

			return true;
		}

		const ElaNewWalletJson &KeyStore::json() const {
			return _walletJson;
		}

		ElaNewWalletJson &KeyStore::json() {
			return _walletJson;
		}

		bool KeyStore::isOld() {
			if (_walletJson.getCoinInfoList().empty()) {
				return true;
			}
			return false;
		}

		bool KeyStore::HasPhrasePassword() const {
			return _walletJson.HasPhrasePassword();
		}

		IAccount *
		KeyStore::createAccountFromJson(const std::string &payPassword) const {
			if (_walletJson.getType() == "Standard" || _walletJson.getType().empty())
				return new StandardAccount(_rootPath, _walletJson.getMnemonic(),
										   _walletJson.getPhrasePassword(), payPassword);
			else if (_walletJson.getType() == "Simple")
				return new SimpleAccount(_walletJson.getPrivateKey(), payPassword);
			else if (_walletJson.getType() == "MultiSign") {

				if (!_walletJson.getMnemonic().empty()) {
					return new MultiSignAccount(
						new StandardAccount(_rootPath, _walletJson.getMnemonic(),
											_walletJson.getPhrasePassword(), payPassword), _walletJson.getCoSigners(),
						_walletJson.getRequiredSignCount());
				} else if (!_walletJson.getPrivateKey().empty()) {
					return new MultiSignAccount(new SimpleAccount(_walletJson.getPrivateKey(), payPassword),
												_walletJson.getCoSigners(), _walletJson.getRequiredSignCount());
				} else {
					return new MultiSignAccount(nullptr, _walletJson.getCoSigners(),
												_walletJson.getRequiredSignCount());
				}
			}

			return nullptr;
		}

		void KeyStore::initJsonFromAccount(IAccount *account, const std::string &payPassword) {
			ParamChecker::checkCondition(account == nullptr, Error::KeyStore, "Account pointer can not be null");
			_walletJson.setType(account->GetType());

			initAccountByType(account, payPassword);
		}

		void KeyStore::initStandardAccount(IAccount *account, const std::string &payPassword) {
			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(account);
			if (standardAccount == nullptr) return;

			std::string phrase;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrase, standardAccount->GetEncryptedMnemonic(), payPassword));
			_walletJson.setMnemonic(phrase);
			_walletJson.setLanguage(standardAccount->GetLanguage());
			std::string phrasePasswd;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(phrasePasswd, standardAccount->GetEncryptedPhrasePassword(),
													   payPassword));
			_walletJson.setPhrasePassword(phrasePasswd);
			if (phrasePasswd.empty()) {
				_walletJson.SetHasPhrasePassword(false);
			} else {
				_walletJson.SetHasPhrasePassword(true);
			}
		}

		void KeyStore::initSimpleAccount(IAccount *account, const std::string &payPassword) {
			SimpleAccount *simpleAccount = dynamic_cast<SimpleAccount *>(account);
			if (simpleAccount == nullptr) return;

			CMBlock privKey;
			ParamChecker::CheckDecrypt(!Utils::Decrypt(privKey, simpleAccount->GetEncryptedKey(), payPassword));
			_walletJson.setPrivateKey(Utils::encodeHex(privKey));
		}

		void KeyStore::initMultiSignAccount(IAccount *account, const std::string &payPassword) {
			MultiSignAccount *multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			if (multiSignAccount == nullptr) return;

			_walletJson.setCoSigners(multiSignAccount->GetCoSigners());
			_walletJson.setRequiredSignCount(multiSignAccount->GetRequiredSignCount());

			if (multiSignAccount->GetInnerAccount() != nullptr)
				initAccountByType(multiSignAccount->GetInnerAccount(), payPassword);
		}

		void KeyStore::initAccountByType(IAccount *account, const std::string &payPassword) {

			if (account->GetType() == "MultiSign") {
				initMultiSignAccount(account, payPassword);
			} else if (account->GetType() == "Simple") {
				initSimpleAccount(account, payPassword);
			} else if (account->GetType() == "Standard") {
				initStandardAccount(account, payPassword);
			}
		}
	}
}