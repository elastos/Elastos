// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		ElaNewWalletJson::ElaNewWalletJson() :
			_singleAddress(false) {

		}

		ElaNewWalletJson::~ElaNewWalletJson() {
			_passphrase.resize(_passphrase.size(), 0);
		}

		void to_json(nlohmann::json &j, const ElaNewWalletJson &p, bool withPrivKey) {
			to_json(j, dynamic_cast<const ElaWebWalletJson &>(p), withPrivKey);

			j["CoinInfoList"] = p._coinInfoList;
			j["SingleAddress"] = p._singleAddress;
		}

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p) {
			from_json(j, dynamic_cast<ElaWebWalletJson &>(p));

			p._old = false;
			if (j.find("CoinInfoList") != j.end()) {
				p._coinInfoList = j["CoinInfoList"].get<std::vector<CoinInfo>>();
			}

			if (j.find("CoSigners") != j.end() && j["Type"] == "MultiSign") {
				p._old = true;
				std::vector<std::string> coSigners;
				coSigners = j["CoSigners"].get<std::vector<std::string>>();
				for (size_t i = 0; i < coSigners.size(); ++i) {
					p.AddPublicKeyRing(PublicKeyRing(coSigners[i]));
				}
				p._singleAddress = true;
			} else if (j.find("IsSingleAddress") != j.end()) {
				p._old = true;
				p._singleAddress = j["IsSingleAddress"];
			}

			if (j.find("RequiredSignCount") != j.end()) {
				p._old = true;
				p.SetM(j["RequiredSignCount"]);
			}

			if (j.find("PhrasePassword") != j.end()) {
				p._old = true;
				p._passphrase = j["PhrasePassword"].get<std::string>();
				if (!p._passphrase.empty())
					p.SetHasPassPhrase(true);
			}

			if (j.find("SingleAddress") != j.end()) {
				p._singleAddress = j["SingleAddress"].get<bool>();
			}
		}

	}
}
