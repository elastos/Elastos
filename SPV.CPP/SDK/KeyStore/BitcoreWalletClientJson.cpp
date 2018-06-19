// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BitcoreWalletClientJson.h"

namespace Elastos {
	namespace ElaWallet {

		BitcoreWalletClientJson::BitcoreWalletClientJson() :
				_m(0),
				_n(0),
				_account(0),
				_mnemonicHasPassphrase(false),
				_compliantDerivation(false) {

		}

		BitcoreWalletClientJson::~BitcoreWalletClientJson() {

		}

		const std::string &BitcoreWalletClientJson::getEncryptedEntropySource() const {
			return _entropySource;
		}

		void BitcoreWalletClientJson::setEncryptedEntropySource(const std::string &entropy) {
			_entropySource = entropy;
		}

		const std::string &BitcoreWalletClientJson::getMnemonic() const {
			return _mnemonic;
		}

		void BitcoreWalletClientJson::setMnemonic(const std::string mnemonic) {
			_mnemonic = mnemonic;
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
			//j["copayerName"] = p.copayerName;
		}

		void from_json(const nlohmann::json &j, BitcoreWalletClientJson::PubKeyItem &p) {
			p.xPubKey = j["xPubKey"].get<std::string>();
			p.requestPubKey = j["requestPubKey"].get<std::string>();
			//p.copayerName = j["copayerName"].get<std::string>();
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
			j["coin"] = p._coin;
			j["network"] = p._network;
			j["xPrivKey"] = p._xPrivKey;
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
			j["mnemonic"] = p._mnemonic;
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
			p._xPrivKey = j["xPrivKey"].get<std::string>();
			p._xPubKey = j["xPubKey"].get<std::string>();
			p._requestPrivKey = j["requestPrivKey"].get<std::string>();
			p._requestPubKey = j["requestPubKey"].get<std::string>();
			p._copayerId = j["copayerId"].get<std::string>();
			p._publicKeyRing = j["publicKeyRing"].get<std::vector<BitcoreWalletClientJson::PubKeyItem>>();
			p._walletId = j["walletId"].get<std::string>();
			p._walletName = j["walletName"].get<std::string>();
			p._m = j["m"].get<int>();
			p._n = j["n"].get<int>();
			p._walletPrivKey = j["walletPrivKey"].get<std::string>();
			p._personalEncryptingKey = j["personalEncryptingKey"].get<std::string>();
			p._sharedEncryptingKey = j["sharedEncryptingKey"].get<std::string>();
			p._copayerName = j["copayerName"].get<std::string>();
			p._mnemonic = j["mnemonic"].get<std::string>();
			p._entropySource = j["entropySource"].get<std::string>();
			p._mnemonicHasPassphrase = j["mnemonicHasPassphrase"].get<bool>();
			p._derivationStrategy = j["derivationStrategy"].get<std::string>();
			p._account = j["account"].get<int>();
			p._compliantDerivation = j["compliantDerivation"].get<bool>();
			p._addressType = j["addressType"].get<std::string>();
		}
	}
}