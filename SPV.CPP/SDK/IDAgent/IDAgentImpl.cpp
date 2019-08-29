// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDAgentImpl.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Implement/MasterWallet.h>
#include <SDK/WalletCore/BIPs/Base58.h>

#include <algorithm>

using namespace nlohmann;

namespace Elastos {
	namespace ElaWallet {

		json &operator<<(json &j, const IDAgentInfo &p) {
			to_json(j, p);

			return j;
		}

		const json &operator>>(const json &j, IDAgentInfo &p) {
			from_json(j, p);

			return j;
		}

		void to_json(json &j, const IDAgentInfo &p) {
			for (IDAgentInfo::IDMap::const_iterator it = p.IDs.cbegin(); it != p.IDs.cend(); ++it) {
				j[it->first] = it->second;
			}
		}

		void from_json(const json &j, IDAgentInfo &p) {
			p.IDs.clear();
			for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
				p.IDs[it.key()] = it.value();
			}
		}

		IDAgentImpl::IDAgentImpl(MasterWallet *parentWallet) :
			_parentWallet(parentWallet) {

		}

		IDAgentImpl::~IDAgentImpl() {

		}

		Address
		IDAgentImpl::DeriveIDAndKeyForPurpose(uint32_t purpose, uint32_t index) {

			ErrorChecker::CheckParam(purpose == 44, Error::DerivePurpose, "Can not use reserved purpose");

			IDItem item(purpose, index);
			std::string existedId;
			if (findIDByPath(item, existedId)) {
				return existedId;
			}

			HDKeychain mpk = _parentWallet->_account->MasterPubKey();

			bytes_t pubKey = mpk.getChild(purpose).getChild(index).pubkey();
			Address address(PrefixIDChain, pubKey);
			item.PublicKey = pubKey;
			_info.IDs[address.String()] = item;
			return address;
		}

		bool IDAgentImpl::IsIDValid(const std::string &id) {
			return Address(id).IsIDAddress();
		}

		bytes_t IDAgentImpl::Sign(const std::string &id, const bytes_t &data, const std::string &passwd) {
			KeyPtr key = generateKey(id, passwd);
			return key->Sign(data);
		}

		std::string IDAgentImpl::Sign(const std::string &id, const std::string &message, const std::string &password) {
			KeyPtr key = generateKey(id, password);
			return key->Sign(message).getHex();
		}

		std::vector<std::string> IDAgentImpl::GetAllIDs() const {
			std::vector<std::string> result;
			std::for_each(_info.IDs.begin(), _info.IDs.end(), [&result](const IDAgentInfo::IDMap::value_type &item) {
				result.push_back(item.first);
			});
			return result;
		}

		KeyPtr IDAgentImpl::generateKey(const std::string &id, const std::string &password) {
			ErrorChecker::CheckCondition(_info.IDs.find(id) == _info.IDs.end(), Error::IDNotFound, std::string("Unknown ID ") + id);
			IDItem item = _info.IDs[id];

			HDKeychain rootKey = _parentWallet->_account->RootKey(password);

			HDKeychain key = rootKey.getChild("44'/0'/0'").getChild(item.Purpose).getChild(item.Index);

			return KeyPtr(new Key(key));
		}

		bool IDAgentImpl::findIDByPath(const IDItem &item, std::string &id) {
			for (IDAgentInfo::IDMap::iterator it = _info.IDs.begin(); it != _info.IDs.end(); it++) {
				if (it->second == item) {
					id = it->first;
					return true;
				}
			}
			return false;
		}

		std::string IDAgentImpl::GenerateRedeemScript(const std::string &id, const std::string &password) {
			ErrorChecker::CheckCondition(_info.IDs.find(id) == _info.IDs.end(), Error::IDNotFound, std::string("Unknown ID ") + id);
			IDItem item = _info.IDs[id];

			HDKeychain mpk = _parentWallet->_account->MasterPubKey();
			bytes_t pubkey = mpk.getChild(item.Purpose).getChild(item.Index).pubkey();
			return Address(PrefixIDChain, pubkey).RedeemScript().getHex();
		}

		const IDAgentInfo &IDAgentImpl::GetIDAgentInfo() const {
			return _info;
		}

		bytes_t IDAgentImpl::GetPublicKey(const std::string &id) const {
			ErrorChecker::CheckCondition(_info.IDs.find(id) == _info.IDs.end(), Error::IDNotFound, "Unknow ID " + id);
			return _info.IDs.find(id)->second.PublicKey;
		}

	}
}