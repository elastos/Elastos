// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson(const std::string &rootPath) :
				_rootPath(rootPath),
				_requiredSignCount(0) {
		}

		ElaNewWalletJson::~ElaNewWalletJson() {

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
			j["CoinInfoList"] = p._coinInfoList;
			j["Type"] = p._type;
			j["Language"] = p._language;
			j["CoSigners"] = p._coSigners;
			j["RequiredSignCount"] = p._requiredSignCount;
			j["PrivateKey"] = p._privateKey;
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			p._coinInfoList = j.find("CoinInfoList") != j.end() ? j["CoinInfoList"].get<std::vector<CoinInfo>>()
																: std::vector<CoinInfo>();
			p._type = j.find("Type") != j.end() ? j["Type"].get<std::string>() : "";
			p._language = j.find("Language") != j.end() ? j["Language"].get<std::string>() : "english";
			p._coSigners = j.find("CoSigners") != j.end() ? j["CoSigners"].get<std::vector<std::string>>()
														  : std::vector<std::string>();
			p._requiredSignCount = j.find("RequiredSignCount") != j.end() ? j["RequiredSignCount"].get<uint32_t>() : 0;
			p._privateKey = j.find("PrivateKey") != j.end() ? j["PrivateKey"].get<std::string>() : "";
		}

		const std::string& ElaNewWalletJson::getType() const {
			return _type;
		}

		void ElaNewWalletJson::setType(const std::string &type) {
			_type = type;
		}

		const std::string &ElaNewWalletJson::getLanguage() const {
			return _language;
		}

		void ElaNewWalletJson::setLanguage(const std::string &la) {
			_language = la;
		}

		const std::vector<std::string> &ElaNewWalletJson::getCoSigners() const {
			return _coSigners;
		}

		void ElaNewWalletJson::setCoSigners(const std::vector<std::string> &coSigners) {
			_coSigners = coSigners;
		}

		uint32_t ElaNewWalletJson::getRequiredSignCount() const {
			return _requiredSignCount;
		}

		void ElaNewWalletJson::setRequiredSignCount(uint32_t count) {
			_requiredSignCount = count;
		}

		const std::string &ElaNewWalletJson::getPrivateKey() const {
			return _privateKey;
		}

		void ElaNewWalletJson::setPrivateKey(const std::string &key) {
			_privateKey = key;
		}

	}
}