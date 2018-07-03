// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>

#include "MasterWalletStore.h"
#include "ParamChecker.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletStore::MasterWalletStore() {

		}

		MasterWalletStore::~MasterWalletStore() {

		}

		void MasterWalletStore::Load(const boost::filesystem::path &path) {
			ParamChecker::checkPathExists(path);

			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;
			j >> *this;
		}

		void MasterWalletStore::Save(const boost::filesystem::path &path) {

			nlohmann::json j;
			j << *this;
			std::ofstream o(path.string());
			j >> o;
		}

		const CMBlock &MasterWalletStore::GetEncrpytedKey() const {
			return _encryptedKey;
		}

		void MasterWalletStore::SetEncryptedKey(const CMBlock &data) {
			_encryptedKey = data;
		}

		const CMBlock &MasterWalletStore::GetEncryptedMnemonic() const {
			return _encryptedMnemonic;
		}

		void MasterWalletStore::SetEncryptedMnemonic(const CMBlock &data) {
			_encryptedMnemonic = data;
		}

		const CMBlock &MasterWalletStore::GetEncrptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		void MasterWalletStore::SetEncryptedPhrasePassword(const CMBlock &data) {
			_encryptedPhrasePass = data;
		}

		const std::string& MasterWalletStore::GetPublicKey() const {
			return _publicKey;
		}

		void MasterWalletStore::SetPublicKey(const std::string &pubKey) {
			_publicKey = pubKey;
		}

		const std::vector<CoinInfo> &MasterWalletStore::GetSubWalletInfoList() const {
			return _subWalletsInfoList;
		}

		void MasterWalletStore::SetSubWalletInfoList(const std::vector<CoinInfo> &infoList) {
			_subWalletsInfoList = infoList;
		}

		const IdAgentInfo &MasterWalletStore::GetIdAgentInfo() const {
			return _idAgentInfo;
		}

		void MasterWalletStore::SetIdAgentInfo(const IdAgentInfo &info) {
			_idAgentInfo = info;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const MasterWalletStore &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, MasterWalletStore &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const MasterWalletStore &p) {
			j["Key"] = Utils::encodeHex(p.GetEncrpytedKey());
			j["Mmemonic"] = Utils::encodeHex(p.GetEncryptedMnemonic());
			j["PhrasePasword"] = Utils::encodeHex(p.GetEncrptedPhrasePassword());
			j["Language"] = p.GetLanguage();
			j["PublicKey"] = p.GetPublicKey();
			j["ChainCode"] = Utils::UInt256ToString(p.GetMasterPubKey().getChainCode());
			j["MasterKeyPubKey"] = Utils::encodeHex(p.GetMasterPubKey().getPubKey());
			j["IdAgent"] = p.GetIdAgentInfo();
			std::vector<nlohmann::json> subWallets;
			for (size_t i = 0; i < p.GetSubWalletInfoList().size(); i++) {
				subWallets.push_back(p.GetSubWalletInfoList()[i]);
			}
			j["SubWallets"] = subWallets;
		}

		void from_json(const nlohmann::json &j, MasterWalletStore &p) {
			p.SetEncryptedKey(Utils::decodeHex(j["Key"].get<std::string>()));
			p.SetEncryptedMnemonic(Utils::decodeHex(j["Mmemonic"].get<std::string>()));
			p.SetEncryptedPhrasePassword(Utils::decodeHex(j["PhrasePasword"].get<std::string>()));
			p.SetLanguage(j["Language"].get<std::string>());
			p.SetPublicKey(j["PublicKey"].get<std::string>());
			UInt256 chainCode = Utils::UInt256FromString(j["ChainCode"].get<std::string>());
			CMBlock pubKey = Utils::decodeHex(j["MasterKeyPubKey"].get<std::string>());
			p.SetMasterPubKey(MasterPubKey(pubKey, chainCode));
			p.SetIdAgentInfo(j["IdAgent"]);

			std::vector<CoinInfo> coinInfoList;
			std::vector<nlohmann::json> subWallets = j["SubWallets"];
			for (size_t i = 0; i < subWallets.size(); i++) {
				coinInfoList.push_back(subWallets[i]);
			}
			p.SetSubWalletInfoList(coinInfoList);
		}

		const std::string &MasterWalletStore::GetLanguage() const {
			return _language;
		}

		void MasterWalletStore::SetLanguage(const std::string &language) {
			_language = language;
		}

		const MasterPubKey &MasterWalletStore::GetMasterPubKey() const {
			return _masterPubKey;
		}

		void MasterWalletStore::SetMasterPubKey(const MasterPubKey &masterPubKey) {
			_masterPubKey = masterPubKey;
		}

	}
}