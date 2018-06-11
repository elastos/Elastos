// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>

#include "MasterWalletStore.h"
#include "ParamChecker.h"

namespace Elastos {
	namespace SDK {

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

		const std::string &MasterWalletStore::GetEncrpytedKey() const {
			return _encryptedKey;
		}

		void MasterWalletStore::SetEncryptedKey(const std::string &data) {
			_encryptedKey = data;
		}

		const std::string &MasterWalletStore::GetEncryptedMnemonic() const {
			return _encryptedMnemonic;
		}

		void MasterWalletStore::SetEncryptedMnemonic(const std::string &data) {
			_encryptedMnemonic = data;
		}

		const std::string& MasterWalletStore::GetEncrptedPhrasePassword() const {
			return _encryptedPhrasePass;
		}

		void MasterWalletStore::SetEncryptedPhrasePassword(const std::string &data) {
			_encryptedPhrasePass = data;
		}

		const std::vector<CoinInfo> & MasterWalletStore::GetSubWalletInfoList() const {
			return _subWalletsInfoList;
		}

		void MasterWalletStore::SetSubWalletInfoList(const std::vector<CoinInfo> &infoList) {
			_subWalletsInfoList = infoList;
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
			j["Key"] = p.GetEncrpytedKey();
			j["Mmemonic"] = p.GetEncryptedMnemonic();
			j["PhrasePasword"] = p.GetEncrptedPhrasePassword();
			std::vector<nlohmann::json> subWallets;
			for(size_t i =0; i < p.GetSubWalletInfoList().size(); i++) {
				subWallets.push_back(p.GetSubWalletInfoList()[i]);
			}
			j["SubWallets"] = subWallets;
		}

		void from_json(const nlohmann::json &j, MasterWalletStore &p) {
			p.SetEncryptedKey(j["Key"].get<std::string>());
			p.SetEncryptedMnemonic(j["Mmemonic"].get<std::string>());
			p.SetEncryptedPhrasePassword(j["PhrasePasword"].get<std::string>());

			std::vector<CoinInfo> coinInfoList;
			std::vector<nlohmann::json> subWallets = j["SubWallets"];
			for(size_t i =0; i < subWallets.size(); i++) {
				coinInfoList.push_back(subWallets[i]);
			}
		}

	}
}