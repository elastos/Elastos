// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BitcoreWalletClientJson.h"

namespace Elastos {
	namespace ElaWallet {

		void to_json(nlohmann::json &j, const PublicKeyRing &p) {
			j = nlohmann::json{
				{"xPubKey", p._xPubKey},
				{"requestPubKey", p._requestPubKey}
			};
		}

		void from_json(const nlohmann::json &j, PublicKeyRing &p) {
			p._xPubKey = j["xPubKey"].get<std::string>();
			p._requestPubKey = j["requestPubKey"].get<std::string>();
		}

		nlohmann::json PublicKeyRing::ToJson() const {
			nlohmann::json j;
			to_json(j, *this);
			return j;
		}

		void PublicKeyRing::FromJson(const nlohmann::json &j) {
			from_json(j, *this);
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
			to_json(j, *this);

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
			from_json(j, *this);
		}

		void to_json(nlohmann::json &j, const BitcoreWalletClientJson &p) {
			j["xPrivKey"] = p._xPrivKey;
			j["coin"] = p._coin;
			j["network"] = p._network;
			j["xPubKey"] = p._xPubKey;
			j["requestPrivKey"] = p._requestPrivKey;
			j["requestPubKey"] = p._requestPubKey;
			j["copayerId"] = p._copayerId;
			j["publicKeyRing"] = p._publicKeyRing;
			j["walletId"] = p._walletId;
			j["walletName"] = p._walletName;
			j["m"] = p._m;
			j["n"] = p._n;
			j["walletPrivKey"] = p._walletPrivKey;
			j["personalEncryptingKey"] = p._personalEncryptingKey;
			j["sharedEncryptingKey"] = p._sharedEncryptingKey;
			j["copayerName"] = p._copayerName;
			j["entropySource"] = p._entropySource;
			j["mnemonicHasPassphrase"] = p._mnemonicHasPassphrase;
			j["derivationStrategy"] = p._derivationStrategy;
			j["account"] = p._account;
			j["compliantDerivation"] = p._compliantDerivation;
			j["addressType"] = p._addressType;
		}

		void from_json(const nlohmann::json &j, BitcoreWalletClientJson &p) {
			p._coin = j.find("coin") != j.end() ? j["coin"].get<std::string>() : "";
			p._network = j.find("network") != j.end() ? j["network"].get<std::string>() : "";
			p._xPrivKey =  j.find("xPrivKey") != j.end() ? j["xPrivKey"].get<std::string>() : "";
			p._xPubKey = j.find("xPubKey") != j.end() ? j["xPubKey"].get<std::string>() : "";
			p._requestPrivKey = j.find("requestPrivKey") != j.end() ? j["requestPrivKey"].get<std::string>() : "";
			p._requestPubKey = j.find("requestPubKey") != j.end() ? j["requestPubKey"].get<std::string>() : "";
			p._copayerId = j.find("copayerId") != j.end() ? j["copayerId"].get<std::string>() : "";
			p._publicKeyRing = j["publicKeyRing"].get<std::vector<PublicKeyRing>>();
			p._walletId = j.find("walletId") != j.end() ? j["walletId"].get<std::string>() : "";
			p._walletName = j.find("walletName") != j.end() ? j["walletName"].get<std::string>() : "";
			p._m = j.find("m") != j.end() ? j["m"].get<int>() : 0;
			p._n = j.find("n") != j.end() ? j["n"].get<int>() : 0;
			p._walletPrivKey = j.find("walletPrivKey") != j.end() ? j["walletPrivKey"].get<std::string>() : "";
			p._personalEncryptingKey = j.find("personalEncryptingKey") != j.end() ? j["personalEncryptingKey"].get<std::string>() : "";
			p._sharedEncryptingKey = j.find("sharedEncryptingKey") != j.end() ? j["sharedEncryptingKey"].get<std::string>() : "";
			p._copayerName = j.find("copayerName") != j.end() ? j["copayerName"].get<std::string>() : "";
			p._entropySource = j.find("entropySource") != j.end() ? j["entropySource"].get<std::string>() : "";
			p._mnemonicHasPassphrase = j.find("mnemonicHasPassphrase") != j.end() ? j["mnemonicHasPassphrase"].get<bool>() : false;
			p._derivationStrategy = j.find("derivationStrategy") != j.end() ? j["derivationStrategy"].get<std::string>() : "";
			p._account = j.find("account") != j.end() ? j["account"].get<int>() : 0;
			p._compliantDerivation = j.find("compliantDerivation") != j.end() ? j["compliantDerivation"].get<bool>() : false;
			p._addressType = j.find("addressType") != j.end() ? j["addressType"].get<std::string>() : "";
		}
	}
}
