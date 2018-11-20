// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <SDK/Wrapper/ByteStream.h>

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
			o << j;
		}

		const std::vector<CoinInfo> &MasterWalletStore::GetSubWalletInfoList() const {
			return _subWalletsInfoList;
		}

		void MasterWalletStore::SetSubWalletInfoList(const std::vector<CoinInfo> &infoList) {
			_subWalletsInfoList = infoList;
		}

		const MasterPubKeyMap &MasterWalletStore::GetMasterPubKeyMap() const {
			return _subWalletsPubKeyMap;
		}

		void MasterWalletStore::SetMasterPubKeyMap(const MasterPubKeyMap &map) {
			_subWalletsPubKeyMap = map;
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

			nlohmann::json masterPubKey;
			const MasterPubKeyMap &map = p.GetMasterPubKeyMap();
			for (MasterPubKeyMap::const_iterator it = map.cbegin(); it != map.cend(); it++) {
				ByteStream stream;
				stream.setPosition(0);
				it->second->Serialize(stream);
				masterPubKey[it->first] = Utils::encodeHex(stream.getBuffer());
			}
			j["MasterPubKey"] = masterPubKey;
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

			MasterPubKeyMap masterPubKeyMap;
			nlohmann::json masterPubKeyJson = j["MasterPubKey"];
			for (nlohmann::json::iterator it = masterPubKeyJson.begin(); it != masterPubKeyJson.end(); ++it) {
				CMBlock value = Utils::decodeHex(it.value());
				MasterPubKeyPtr masterPubKey = MasterPubKeyPtr(new MasterPubKey());
				ByteStream stream(value, value.GetSize(), false);
				masterPubKey->Deserialize(stream);
				masterPubKeyMap[it.key()] = masterPubKey;
			}
			p.SetMasterPubKeyMap(masterPubKeyMap);
		}

		IAccount *MasterWalletStore::Account() const {
			return _account.get();
		}

		void MasterWalletStore::Reset(Elastos::ElaWallet::IAccount *account) {
			_account = AccountPtr(account);
		}

		void MasterWalletStore::Reset(const std::string &phrase,
									  const std::string &phrasePassword, const std::string &payPassword) {
			_account = AccountPtr(new StandardAccount(_rootPath, phrase, phrasePassword, payPassword));
		}

		void MasterWalletStore::Reset(const nlohmann::json &coSigners, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(nullptr, coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &privKey, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(
					new MultiSignAccount(new SimpleAccount(privKey, payPassword), coSigners, requiredSignCount));
		}

		void MasterWalletStore::Reset(const std::string &phrase,
									  const std::string &phrasePassword, const nlohmann::json &coSigners,
									  const std::string &payPassword, uint32_t requiredSignCount) {
			_account = AccountPtr(new MultiSignAccount(
					new StandardAccount(_rootPath, phrase, phrasePassword, payPassword),
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