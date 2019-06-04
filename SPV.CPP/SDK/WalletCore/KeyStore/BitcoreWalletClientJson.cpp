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

			if (!withPrivKey)
				j.erase("xPrivKey");

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
			p._coin = j["coin"].get<std::string>();
			p._network = j["network"].get<std::string>();
			p._xPrivKey =  j.find("xPrivKey") != j.end() ? j["xPrivKey"].get<std::string>() : "";
			p._xPubKey = j["xPubKey"].get<std::string>();
			p._requestPrivKey = j.find("requestPrivKey") != j.end() ? j["requestPrivKey"].get<std::string>() : "";
			p._requestPubKey = j["requestPubKey"].get<std::string>();
			p._copayerId = j["copayerId"].get<std::string>();
			p._publicKeyRing = j["publicKeyRing"].get<std::vector<PublicKeyRing>>();
			p._walletId = j["walletId"].get<std::string>();
			p._walletName = j["walletName"].get<std::string>();
			p._m = j["m"].get<int>();
			p._n = j["n"].get<int>();
			p._walletPrivKey = j["walletPrivKey"].get<std::string>();
			p._personalEncryptingKey = j["personalEncryptingKey"].get<std::string>();
			p._sharedEncryptingKey = j["sharedEncryptingKey"].get<std::string>();
			p._copayerName = j["copayerName"].get<std::string>();
			p._entropySource = j["entropySource"].get<std::string>();
			p._mnemonicHasPassphrase = j["mnemonicHasPassphrase"].get<bool>();
			p._derivationStrategy = j["derivationStrategy"].get<std::string>();
			p._account = j["account"].get<int>();
			p._compliantDerivation = j["compliantDerivation"].get<bool>();
			p._addressType = j["addressType"].get<std::string>();
		}
	}
}
