// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace SDK {

		ElaNewWalletJson::ElaNewWalletJson() :
				_id(""),
				_idInfo(""),
				_mnemonicLanguage("english") {
		}

		ElaNewWalletJson::~ElaNewWalletJson() {

		}

		const std::string &ElaNewWalletJson::getID() const {
			return _id;
		}

		void ElaNewWalletJson::setID(const std::string &id) {
			_id = id;
		}

		const std::string &ElaNewWalletJson::getIDInfo() const {
			return _idInfo;
		}

		void ElaNewWalletJson::setIDInfo(const std::string &value) {
			_idInfo = value;
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
			j["id"] = p._id;
			j["idInfo"] = p._idInfo;
			j["mnemonicLanguage"] = p._mnemonicLanguage;
			j["encryptedPhrasePassword"] = p._encryptedPhrasePassword;
			j["coinInfoList"] = p._coinInfoList;
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			p._id = j["id"].get<std::string>();
			p._idInfo = j["idInfo"].get<std::string>();
			p._mnemonicLanguage = j["mnemonicLanguage"].get<std::string>();
			p._encryptedPhrasePassword = j["encryptedPhrasePassword"].get<std::string>();
			std::vector<nlohmann::json> infoListRaw = j["coinInfoList"].get<std::vector<nlohmann::json>>();
			for (size_t i = 0; i < infoListRaw.size(); i++) {
				CoinInfo info;
				info = infoListRaw[i];
				p._coinInfoList.push_back(info);
			}
		}

		const std::string &ElaNewWalletJson::getEncryptedPhrasePassword() const {
			return _encryptedPhrasePassword;
		}

		void ElaNewWalletJson::setEncryptedPhrasePassword(const std::string &password) {
			_encryptedPhrasePassword = password;
		}
	}
}