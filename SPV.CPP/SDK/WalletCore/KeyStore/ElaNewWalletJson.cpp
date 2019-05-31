// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"
#include "CoinInfo.h"

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson() :
			_singleAddress(false) {

		}

		ElaNewWalletJson::~ElaNewWalletJson() {
			_passphrase.resize(_passphrase.size(), 0);
		}

		nlohmann::json ElaNewWalletJson::ToJson(bool withPrivKey) const {
			nlohmann::json j = ElaWebWalletJson::ToJson(withPrivKey);
			ToJsonCommon(j);
			return j;
		}

		void ElaNewWalletJson::FromJson(const nlohmann::json &j) {
			ElaWebWalletJson::FromJson(j);
			FromJsonCommon(j);
		}

		void to_json(nlohmann::json &j, const ElaNewWalletJson &p) {
			to_json(j, dynamic_cast<const ElaWebWalletJson &>(p));
			p.ToJsonCommon(j);
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			from_json(j, dynamic_cast<ElaWebWalletJson &>(p));
			p.FromJsonCommon(j);
		}

		void ElaNewWalletJson::ToJsonCommon(nlohmann::json &j) const {
			nlohmann::json coinInfoList;
			for (size_t i = 0; i < _coinInfoList.size(); ++i)
				coinInfoList.push_back(_coinInfoList[i]->ToJson());

			j["CoinInfoList"] = coinInfoList;
			j["SingleAddress"] = _singleAddress;
		}

		void ElaNewWalletJson::FromJsonCommon(const nlohmann::json &j) {

			_old = false;
			if (j.find("CoinInfoList") != j.end()) {
				_coinInfoList.clear();
				nlohmann::json coinInfoList = j["CoinInfoList"];
				for (nlohmann::json::iterator it = coinInfoList.begin(); it != coinInfoList.end(); ++it) {
					CoinInfoPtr coinInfo(new CoinInfo());
					coinInfo->FromJson((*it));
					_coinInfoList.push_back(coinInfo);
				}
			}

			if (j.find("CoSigners") != j.end() && j["Type"] == "MultiSign") {
				_old = true;
				std::vector<std::string> coSigners;
				coSigners = j["CoSigners"].get<std::vector<std::string>>();
				for (size_t i = 0; i < coSigners.size(); ++i) {
					AddPublicKeyRing(PublicKeyRing(coSigners[i]));
				}
				SetN(coSigners.size());
				_singleAddress = true;
			} else if (j.find("IsSingleAddress") != j.end()) {
				_old = true;
				_singleAddress = j["IsSingleAddress"];
			}

			if (j.find("RequiredSignCount") != j.end()) {
				_old = true;
				SetM(j["RequiredSignCount"]);
			}

			if (j.find("PhrasePassword") != j.end()) {
				_old = true;
				_passphrase = j["PhrasePassword"].get<std::string>();
				if (!_passphrase.empty())
					SetHasPassPhrase(true);
			}

			if (j.find("SingleAddress") != j.end()) {
				_singleAddress = j["SingleAddress"].get<bool>();
			}
		}

	}
}
