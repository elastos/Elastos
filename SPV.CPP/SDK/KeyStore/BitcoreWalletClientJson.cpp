// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BitcoreWalletClientJson.h"

namespace Elastos {
	namespace SDK {

		BitcoreWalletClientJson::BitcoreWalletClientJson() {

		}

		BitcoreWalletClientJson::~BitcoreWalletClientJson() {

		}

		nlohmann::json &operator<<(nlohmann::json &j, const BitcoreWalletClientJson::PubKeyItem &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, BitcoreWalletClientJson::PubKeyItem &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const BitcoreWalletClientJson::PubKeyItem &p) {
			j["xPubKey"] = p.xPubKey;
			j["requestPubKey"] = p.requestPubKey;
		}

		void from_json(const nlohmann::json &j, BitcoreWalletClientJson::PubKeyItem &p) {
			p.xPubKey = j["xPubKey"].get<std::string>();
			p.requestPubKey = j["requestPubKey"].get<std::string>();
		}

		nlohmann::json &operator<<(nlohmann::json &j, const BitcoreWalletClientJson &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, BitcoreWalletClientJson &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const BitcoreWalletClientJson &p) {
			j["_coin"] = p._coin;
			j["_network"] = p._network;
			j["_xPrivKey"] = p._xPrivKey;
			j["_pubKeyItem"] = p._pubKeyItem;
			j["_publicKeyRing"]= p._publicKeyRing;
			j["_requestPrivKey"] = p._requestPrivKey;
			j["_copayerId"] = p._copayerId;
			j["_walletId"] = p._walletId;
			j["_walletName"] = p._walletName;
			j["_m"] = p._m;
			j["_n"] = p._n;
			j["_walletPrivKey"] = p._walletPrivKey;
			j["_personalEncryptingKey"] = p._personalEncryptingKey;
			j["_sharedEncryptingKey"] = p._sharedEncryptingKey;
			j["_copayerName"] = p._copayerName;
			j["_entropySource"] = p._entropySource;
			j["_derivationStrategy"] = p._derivationStrategy;
			j["_account"] = p._account;
			j["_compliantDerivation"] = p._compliantDerivation;
			j["_addressType"] = p._addressType;
		}

		void from_json(const nlohmann::json &j, BitcoreWalletClientJson &p) {
			p._coin = j["_coin"].get<std::string>();
			p._network = j["_network"].get<std::string>();
			p._xPrivKey = j["_xPrivKey"].get<std::string>();
			p._pubKeyItem = j["_pubKeyItem"].get<BitcoreWalletClientJson::PubKeyItem>();
			p._publicKeyRing = j["_publicKeyRing"].get<std::vector<BitcoreWalletClientJson::PubKeyItem>>();
			p._requestPrivKey = j["_requestPrivKey"].get<std::string>();
			p._copayerId = j["_copayerId"].get<std::string>();
			p._walletId = j["_walletId"].get<std::string>();
			p._walletName = j["_walletName"].get<std::string>();
			p._m = j["_m"].get<int>();
			p._n = j["_n"].get<int>();
			p._walletPrivKey = j["_walletPrivKey"].get<std::string>();
			p._personalEncryptingKey = j["_personalEncryptingKey"].get<std::string>();
			p._sharedEncryptingKey = j["_sharedEncryptingKey"].get<std::string>();
			p._copayerName = j["_copayerName"].get<std::string>();
			p._entropySource = j["_entropySource"].get<std::string>();
			p._derivationStrategy = j["_derivationStrategy"].get<std::string>();
			p._account = j["_account"].get<int>();
			p._compliantDerivation = j["_compliantDerivation"].get<bool>();
			p._addressType = j["_addressType"].get<std::string>();
		}
	}
}