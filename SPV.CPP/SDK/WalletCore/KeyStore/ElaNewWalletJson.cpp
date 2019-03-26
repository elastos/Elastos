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

		void ElaNewWalletJson::AddCoinInfo(const CoinInfo &info) {
			_coinInfoList.push_back(info);
		}

		void ElaNewWalletJson::ClearCoinInfo() {
			_coinInfoList.clear();
		}

		const std::vector<CoinInfo> &ElaNewWalletJson::GetCoinInfoList() const {
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
			j["PhrasePassword"] = p._phrasePassword;
			j["IsSingleAddress"] = p._isSingleAddress;
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
			p._phrasePassword = j.find("PhrasePassword") != j.end() ? j["PhrasePassword"].get<std::string>() : "";
			p._isSingleAddress = j.find("IsSingleAddress") != j.end() ? j["IsSingleAddress"].get<bool>() : false;
		}

		const std::string& ElaNewWalletJson::GetType() const {
			return _type;
		}

		void ElaNewWalletJson::SetType(const std::string &type) {
			_type = type;
		}

		const std::string &ElaNewWalletJson::GetLanguage() const {
			return _language;
		}

		void ElaNewWalletJson::SetLanguage(const std::string &la) {
			_language = la;
		}

		const std::vector<std::string> &ElaNewWalletJson::GetCoSigners() const {
			return _coSigners;
		}

		void ElaNewWalletJson::SetCoSigners(const std::vector<std::string> &coSigners) {
			_coSigners = coSigners;
		}

		uint32_t ElaNewWalletJson::GetRequiredSignCount() const {
			return _requiredSignCount;
		}

		void ElaNewWalletJson::SetRequiredSignCount(uint32_t count) {
			_requiredSignCount = count;
		}

		const std::string &ElaNewWalletJson::GetPrivateKey() const {
			return _privateKey;
		}

		void ElaNewWalletJson::SetPrivateKey(const std::string &key) {
			_privateKey = key;
		}

		const std::string &ElaNewWalletJson::GetPhrasePassword() const {
			return _phrasePassword;
		}

		void ElaNewWalletJson::SetPhrasePassword(const std::string &phrasePassword) {
			_phrasePassword = phrasePassword;
		}

		void ElaNewWalletJson::SetIsSingleAddress(bool value) {
			_isSingleAddress = value;
		}

		bool ElaNewWalletJson::GetIsSingleAddress() const {
			return _isSingleAddress;
		}

	}
}