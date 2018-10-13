// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>

#include "MasterWalletStore.h"
#include "ParamChecker.h"
#include "Utils.h"
#include "SDK/Account/StandardAccount.h"
#include "SDK/Account/MultiSignAccount.h"
#include "SDK/Account/SimpleAccount.h"
#include "AccountFactory.h"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletStore::MasterWalletStore(const std::string &rootPath) : _rootPath(rootPath) {
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
			j["Account"] = p.Account()->ToJson();
			j["AccountType"] = p.Account()->GetType();
			j["IsSingleAddress"] = p.IsSingleAddress();
			j["IdAgent"] = p.GetIdAgentInfo();
			std::vector<nlohmann::json> subWallets;
			for (size_t i = 0; i < p.GetSubWalletInfoList().size(); i++) {
				subWallets.push_back(p.GetSubWalletInfoList()[i]);
			}
			j["SubWallets"] = subWallets;
		}

		void from_json(const nlohmann::json &j, MasterWalletStore &p) {
			p._account = AccountPtr(AccountFactory::CreateFromJson(p._rootPath, j));
			p.SetIdAgentInfo(j["IdAgent"]);
			p.IsSingleAddress() = j["IsSingleAddress"].get<bool>();

			std::vector<CoinInfo> coinInfoList;
			std::vector<nlohmann::json> subWallets = j["SubWallets"];
			for (size_t i = 0; i < subWallets.size(); i++) {
				coinInfoList.push_back(subWallets[i]);
			}
			p.SetSubWalletInfoList(coinInfoList);
		}

		IAccount *MasterWalletStore::Account() const {
			return _account.get();
		}

		void MasterWalletStore::Reset(Elastos::ElaWallet::IAccount *account) {
			_account = AccountPtr(account);
		}

		void MasterWalletStore::Reset(const std::string &phrase, const std::string &language,
									  const std::string &phrasePassword, const std::string &payPassword) {
			_account = AccountPtr(new StandardAccount(_rootPath, phrase, language, phrasePassword, payPassword));
		}

		void MasterWalletStore::Reset(const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(nullptr, coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &privKey, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(
					new MultiSignAccount(new SimpleAccount(privKey, payPassword), coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &phrase, const std::string &language,
									  const std::string &phrasePassword, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(
					new StandardAccount(_rootPath, phrase, language, phrasePassword, payPassword),
					coSigners, requiredSignCount));
		}

		bool MasterWalletStore::IsSingleAddress() const {
			return _isSingleAddress;
		}

		bool &MasterWalletStore::IsSingleAddress() {
			return _isSingleAddress;
		}
	}
}