// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BIPs/HDKeychain.h>
#include <BIPs/BIP39.h>
#include <BIPs/Base58.h>
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
			j["OwnerPubKey"] = _ownerPubKey;
		}

		void ElaNewWalletJson::FromJsonCommon(const nlohmann::json &j) {

			bool old = false;

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
				old = true;
				std::vector<std::string> coSigners;
				coSigners = j["CoSigners"].get<std::vector<std::string>>();
				for (size_t i = 0; i < coSigners.size(); ++i) {
					_publicKeyRing.emplace_back(coSigners[i]);
				}
				_n = coSigners.size();
				_singleAddress = true;
			}

			if (j.find("RequiredSignCount") != j.end()) {
				old = true;
				_m = j["RequiredSignCount"].get<int>();
				if (_m == 0)
					_m = 1;
			}

			if (j.find("PhrasePassword") != j.end()) {
				old = true;
				_passphrase = j["PhrasePassword"].get<std::string>();
				if (!_passphrase.empty())
					_mnemonicHasPassphrase = true;
			}

			if (j.find("IsSingleAddress") != j.end()) {
				old = true;
				_singleAddress = j["IsSingleAddress"];
			}

			if (j.find("SingleAddress") != j.end()) {
				_singleAddress = j["SingleAddress"].get<bool>();
			}

			if (j.find("OwnerPubKey") != j.end()) {
				_ownerPubKey = j["OwnerPubKey"].get<std::string>();
			}

			if (!_mnemonic.empty() && old) {
				HDSeed seed(BIP39::DeriveSeed(_mnemonic, _passphrase).bytes());
				HDKeychain rootkey(seed.getExtendedKey(true));

				_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();

				_xPrivKey = Base58::CheckEncode(rootkey.extkey());
				_xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

				HDKeychain requestKey = rootkey.getChild("1'/0");
				_requestPrivKey = requestKey.privkey().getHex();
				_requestPubKey = requestKey.pubkey().getHex();

				_publicKeyRing.emplace_back(_requestPubKey, _xPubKey);
				if (_m == 0)
					_m = 1;
				_n = _publicKeyRing.size();

				_passphrase.clear();
			}

			if (_ownerPubKey.empty() && !_xPrivKey.empty()) {
				bytes_t bytes;
				Base58::CheckDecode(_xPrivKey, bytes);
				HDKeychain rootkey(bytes);
				_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();
			}

			if (_n > 1) {
				_singleAddress = true;
			}
		}

	}
}
