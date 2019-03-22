// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdAgentImpl.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Implement/MasterWallet.h>
#include <SDK/Account/StandardAccount.h>

#include <algorithm>
#include <SDK/BIPs/Base58.h>

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

		Address
		IdAgentImpl::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index) {

			ErrorChecker::CheckParam(purpose == 44, Error::DerivePurpose, "Can not use reserved purpose");

			IdItem item(purpose, index);
			std::string existedId;
			if (findIdByPath(item, existedId)) {
				return existedId;
			}

			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(_parentWallet->_localStore.Account());
			ErrorChecker::CheckCondition(standardAccount == nullptr, Error::WrongAccountType,
										 "This account can not create ID");

			const HDKeychain &mpk = standardAccount->GetIDMasterPubKey();

			bytes_t pubKey = mpk.getChild(purpose).getChild(index).pubkey();
			Address address(PrefixIDChain, pubKey);
			item.PublicKey = pubKey;
			_info.Ids[address.String()] = item;
			return address;
		}

		bool IdAgentImpl::IsIdValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		bytes_t IdAgentImpl::Sign(const std::string &id, const bytes_t &data, const std::string &passwd) {
			KeyPtr key = generateKey(id, passwd);
			return key->Sign(data);
		}

		std::string IdAgentImpl::Sign(const std::string &id, const std::string &message, const std::string &password) {
			KeyPtr key = generateKey(id, password);
			return key->Sign(message).getHex();
		}

		std::vector<std::string> IdAgentImpl::GetAllIds() const {
			std::vector<std::string> result;
			std::for_each(_info.Ids.begin(), _info.Ids.end(), [&result](const IdAgentInfo::IdMap::value_type &item) {
				result.push_back(item.first);
			});
			return result;
		}

		KeyPtr IdAgentImpl::generateKey(const std::string &id, const std::string &password) {
			ErrorChecker::CheckCondition(_info.Ids.find(id) == _info.Ids.end(), Error::IDNotFound, std::string("Unknown ID ") + id);
			IdItem item = _info.Ids[id];

			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(_parentWallet->_localStore.Account());
			ErrorChecker::CheckCondition(standardAccount == nullptr, Error::WrongAccountType,
										 "This account can not create ID");


			HDSeed hdseed(standardAccount->DeriveSeed(password).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			HDKeychain key = rootKey.getChild("44'/0'/0'").getChild(item.Purpose).getChild(item.Index);

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
			ErrorChecker::CheckCondition(_info.Ids.find(id) == _info.Ids.end(), Error::IDNotFound, std::string("Unknown ID ") + id);
			IdItem item = _info.Ids[id];

			StandardAccount *standardAccount = dynamic_cast<StandardAccount *>(_parentWallet->_localStore.Account());
			ErrorChecker::CheckCondition(standardAccount == nullptr, Error::WrongAccountType,
										 "This account can not create ID");

			bytes_t pubkey = standardAccount->GetIDMasterPubKey().getChild(item.Purpose).getChild(item.Index).pubkey();
			return Address(PrefixIDChain, pubkey).RedeemScript().getHex();
		}

		const IdAgentInfo &IdAgentImpl::GetIdAgentInfo() const {
			return _info;
		}

		bytes_t IdAgentImpl::GetPublicKey(const std::string &id) const {
			ErrorChecker::CheckCondition(_info.Ids.find(id) == _info.Ids.end(), Error::IDNotFound, "Unknow ID " + id);
			return _info.Ids.find(id)->second.PublicKey;
		}

	}
}