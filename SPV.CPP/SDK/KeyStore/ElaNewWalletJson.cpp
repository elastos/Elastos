// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson() :
			_mnemonicLanguage("english") {
		}

		ElaNewWalletJson::~ElaNewWalletJson() {

		}

		const std::string &ElaNewWalletJson::getMnemonicLanguage() const {
			return _mnemonicLanguage;
		}

		void ElaNewWalletJson::setMnemonicLanguage(const std::string &language) {
			_mnemonicLanguage = language;
		}

		void ElaNewWalletJson::addCoinInfo(const CoinInfo &info) {
			_coinInfoList.push_back(info);
		}

		void ElaNewWalletJson::clearCoinInfo() {
			_coinInfoList.clear();
		}

		const std::vector<CoinInfo> &ElaNewWalletJson::getCoinInfoList() const {
			return _coinInfoList;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const ElaNewWalletJson &p) {
			j << *(ElaWebWalletJson *) &p;

			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, ElaNewWalletJson &p) {
			j >> *(ElaWebWalletJson *) &p;

			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const ElaNewWalletJson &p) {
			j["mnemonicLanguage"] = p._mnemonicLanguage;
			j["encryptedPhrasePassword"] = p._encryptedPhrasePassword;
			j["coinInfoList"] = p._coinInfoList;
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			p._mnemonicLanguage =
				j.find("mnemonicLanguage") != j.end() ? j["mnemonicLanguage"].get<std::string>() : p._mnemonicLanguage;
			p._encryptedPhrasePassword =
				j.find("encryptedPhrasePassword") != j.end() ? j["encryptedPhrasePassword"].get<std::string>() : "";
			std::vector<CoinInfo> tmp;
			p._coinInfoList = j.find("coinInfoList") != j.end() ? j["coinInfoList"].get<std::vector<CoinInfo>>() : tmp;
		}

		const std::string &ElaNewWalletJson::getEncryptedPhrasePassword() const {
			return _encryptedPhrasePassword;
		}

		void ElaNewWalletJson::setEncryptedPhrasePassword(const std::string &password) {
			_encryptedPhrasePassword = password;
		}
	}
}