// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BitcoreWalletClientJson.h"
#include <Common/JsonSerializer.h>

namespace Elastos {
	namespace ElaWallet {

		nlohmann::json PublicKeyRing::ToJson() const {
			nlohmann::json j = nlohmann::json {
				{"xPubKey", _xPubKey},
				{"requestPubKey", _requestPubKey}
			};
			return j;
		}

		void PublicKeyRing::FromJson(const nlohmann::json &j) {
			_xPubKey = j["xPubKey"].get<std::string>();
			_requestPubKey = j["requestPubKey"].get<std::string>();
		}

		BitcoreWalletClientJson::BitcoreWalletClientJson() :
				_m(0),
				_n(0),
				_account(0),
				_mnemonicHasPassphrase(false),
				_compliantDerivation(false) {

		}

		BitcoreWalletClientJson::~BitcoreWalletClientJson() {
			_xPrivKey.resize(_xPrivKey.size(), 0);
			_requestPrivKey.resize(_requestPrivKey.size(), 0);
		}

		nlohmann::json BitcoreWalletClientJson::ToJson(bool withPrivKey) const {
			nlohmann::json j;
			j["xPrivKey"] = _xPrivKey;
			j["coin"] = _coin;
			j["network"] = _network;
			j["xPubKey"] = _xPubKey;
			j["requestPrivKey"] = _requestPrivKey;
			j["requestPubKey"] = _requestPubKey;
			j["copayerId"] = _copayerId;
			j["publicKeyRing"] = _publicKeyRing;
			j["walletId"] = _walletId;
			j["walletName"] = _walletName;
			j["m"] = _m;
			j["n"] = _n;
			j["walletPrivKey"] = _walletPrivKey;
			j["personalEncryptingKey"] = _personalEncryptingKey;
			j["sharedEncryptingKey"] = _sharedEncryptingKey;
			j["copayerName"] = _copayerName;
			j["entropySource"] = _entropySource;
			j["mnemonicHasPassphrase"] = _mnemonicHasPassphrase;
			j["derivationStrategy"] = _derivationStrategy;
			j["account"] = _account;
			j["compliantDerivation"] = _compliantDerivation;
			j["addressType"] = _addressType;

			if (!withPrivKey) {
				j.erase("xPrivKey");
				j.erase("requestPrivKey");
				j.erase("coin");
				j.erase("account");
				j.erase("derivationStrategy");
				j.erase("addressType");
				j.erase("copayerId");
				j.erase("copayerName");
				j.erase("entropySource");
				j.erase("personalEncryptingKey");
				j.erase("walletPrivKey");
				j.erase("walletName");
				j.erase("walletId");
				j.erase("sharedEncryptingKey");
				j.erase("compliantDerivation");
			}

			return j;
		}

		void BitcoreWalletClientJson::FromJson(const nlohmann::json &j) {
			_coin = j.find("coin") != j.end() ? j["coin"].get<std::string>() : "";
			_network = j.find("network") != j.end() ? j["network"].get<std::string>() : "";
			_xPrivKey =  j.find("xPrivKey") != j.end() ? j["xPrivKey"].get<std::string>() : "";
			_xPubKey = j.find("xPubKey") != j.end() ? j["xPubKey"].get<std::string>() : "";
			_requestPrivKey = j.find("requestPrivKey") != j.end() ? j["requestPrivKey"].get<std::string>() : "";
			_requestPubKey = j.find("requestPubKey") != j.end() ? j["requestPubKey"].get<std::string>() : "";
			_copayerId = j.find("copayerId") != j.end() ? j["copayerId"].get<std::string>() : "";
			_publicKeyRing = j["publicKeyRing"].get<std::vector<PublicKeyRing>>();
			_walletId = j.find("walletId") != j.end() ? j["walletId"].get<std::string>() : "";
			_walletName = j.find("walletName") != j.end() ? j["walletName"].get<std::string>() : "";
			_m = j.find("m") != j.end() ? j["m"].get<int>() : 0;
			_n = j.find("n") != j.end() ? j["n"].get<int>() : 0;
			_walletPrivKey = j.find("walletPrivKey") != j.end() ? j["walletPrivKey"].get<std::string>() : "";
			_personalEncryptingKey = j.find("personalEncryptingKey") != j.end() ? j["personalEncryptingKey"].get<std::string>() : "";
			_sharedEncryptingKey = j.find("sharedEncryptingKey") != j.end() ? j["sharedEncryptingKey"].get<std::string>() : "";
			_copayerName = j.find("copayerName") != j.end() ? j["copayerName"].get<std::string>() : "";
			_entropySource = j.find("entropySource") != j.end() ? j["entropySource"].get<std::string>() : "";
			_mnemonicHasPassphrase = j.find("mnemonicHasPassphrase") != j.end() ? j["mnemonicHasPassphrase"].get<bool>() : false;
			_derivationStrategy = j.find("derivationStrategy") != j.end() ? j["derivationStrategy"].get<std::string>() : "";
			_account = j.find("account") != j.end() ? j["account"].get<int>() : 0;
			_compliantDerivation = j.find("compliantDerivation") != j.end() ? j["compliantDerivation"].get<bool>() : false;
			_addressType = j.find("addressType") != j.end() ? j["addressType"].get<std::string>() : "";
		}

	}
}
