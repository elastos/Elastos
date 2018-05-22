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
			j["_id"] = p._id;
			j["_idInfo"] = p._idInfo;
			j["_mnemonicLanguage"] = p._mnemonicLanguage;
			j["_coinInfoList"] = p._coinInfoList;
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			p._id = j["_id"].get<std::string>();
			p._idInfo = j["_idInfo"].get<std::string>();
			p._mnemonicLanguage = j["_mnemonicLanguage"].get<std::string>();
			p._coinInfoList = j["_coinInfoList"].get<std::vector<CoinInfo>>();
		}
	}
}