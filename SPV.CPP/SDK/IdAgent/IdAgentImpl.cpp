// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdAgentImpl.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Implement/MasterWallet.h>
#include <SDK/Account/StandardAccount.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <algorithm>
#include <SDK/Common/Base58.h>

using namespace nlohmann;

namespace Elastos {
	namespace ElaWallet {

		json &operator<<(json &j, const IdAgentInfo &p) {
			to_json(j, p);

			return j;
		}

		const json &operator>>(const json &j, IdAgentInfo &p) {
			from_json(j, p);

			return j;
		}

		void to_json(json &j, const IdAgentInfo &p) {
			for (IdAgentInfo::IdMap::const_iterator it = p.Ids.cbegin(); it != p.Ids.cend(); ++it) {
				j[it->first] = it->second;
			}
		}

		void from_json(const json &j, IdAgentInfo &p) {
			p.Ids.clear();
			for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
				p.Ids[it.key()] = it.value();
			}
		}

		IdAgentImpl::IdAgentImpl(MasterWallet *parentWallet, const IdAgentInfo &info) :
			_parentWallet(parentWallet),
			_info(info) {

		}

		IdAgentImpl::~IdAgentImpl() {

		}

		std::string
		IdAgentImpl::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index) {

			ParamChecker::checkParam(purpose == 44, Error::DerivePurpose, "Can not use reserved purpose");

			IdItem item(purpose, index);
			std::string existedId;
			if (findIdByPath(item, existedId)) {
				return existedId;
			}

			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(_parentWallet->_localStore.Account());
			ParamChecker::checkCondition(standardAccount == nullptr, Error::WrongAccountType,
										 "This account can not create ID");

			const MasterPubKey &mpk = standardAccount->GetIDMasterPubKey();

			CMBlock pubKey = BIP32Sequence::PubKey(mpk, purpose, index);

			Key key;
			key.SetPubKey(pubKey);
			std::string id = key.GetAddress(PrefixIDChain);
			item.PublicKey = Utils::encodeHex(pubKey);
			_info.Ids[id] = item;

			return id;
		}

		bool IdAgentImpl::IsIdValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		CMBlock IdAgentImpl::Sign(const std::string &id, const CMBlock &data, const std::string &passwd) {
			KeyPtr key = generateKey(id, passwd);
			return key->Sign(data);
		}

		std::string IdAgentImpl::Sign(const std::string &id, const std::string &message, const std::string &password) {
			KeyPtr key = generateKey(id, password);
			return Utils::encodeHex(key->Sign(message));
		}

		std::vector<std::string> IdAgentImpl::GetAllIds() const {
			std::vector<std::string> result;
			std::for_each(_info.Ids.begin(), _info.Ids.end(), [&result](const IdAgentInfo::IdMap::value_type &item) {
				result.push_back(item.first);
			});
			return result;
		}

		KeyPtr IdAgentImpl::generateKey(const std::string &id, const std::string &password) {
			ParamChecker::checkCondition(_info.Ids.find(id) == _info.Ids.end(), Error::IDNotFound, "Unknown ID " + id);
			IdItem item = _info.Ids[id];

			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(_parentWallet->_localStore.Account());
			ParamChecker::checkCondition(standardAccount == nullptr, Error::WrongAccountType,
										 "This account can not create ID");

			UInt512 seed = standardAccount->DeriveSeed(password);
			UInt256 chainCode;
			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 3, 0 | BIP32_HARD, item.Purpose,
												 item.Index);
			var_clean(&seed);
			return KeyPtr(new Key(key));
		}

		bool IdAgentImpl::findIdByPath(const IdItem &item, std::string &id) {
			for (IdAgentInfo::IdMap::iterator it = _info.Ids.begin(); it != _info.Ids.end(); it++) {
				if (it->second == item) {
					id = it->first;
					return true;
				}
			}
			return false;
		}

		std::string IdAgentImpl::GenerateRedeemScript(const std::string &id, const std::string &password) {
			KeyPtr key = generateKey(id, password);
			return Utils::encodeHex(key->RedeemScript(PrefixIDChain));
		}

		const IdAgentInfo &IdAgentImpl::GetIdAgentInfo() const {
			return _info;
		}

		std::string IdAgentImpl::GetPublicKey(const std::string &id) const {
			ParamChecker::checkCondition(_info.Ids.find(id) == _info.Ids.end(), Error::IDNotFound, "Unknow ID " + id);
			return _info.Ids.find(id)->second.PublicKey;
		}

	}
}