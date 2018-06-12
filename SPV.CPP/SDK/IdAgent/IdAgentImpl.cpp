// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <algorithm>
#include <SDK/Common/ParamChecker.h>

#include "IdAgentImpl.h"
#include "MasterWallet.h"
#include "Wrapper/Address.h"

using namespace nlohmann;

namespace Elastos {
	namespace SDK {

		json &operator<<(json &j, const IdAgentInfo &p) {
			to_json(j, p);

			return j;
		}

		const json &operator>>(const json &j, IdAgentInfo &p) {
			from_json(j, p);

			return j;
		}

		void to_json(json &j, const IdAgentInfo &p) {
			for(IdAgentInfo::IdMap::const_iterator it = p.Ids.cbegin(); it != p.Ids.cend(); ++it) {
				std::vector<uint32_t> values = {it->second.Purpose, it->second.Index};
				j[it->first] = values;
			}
		}

		void from_json(const json &j, IdAgentInfo &p) {
			p.Ids.clear();
			for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
				p.Ids[it.key()] = IdPath(it.value().at(0), it.value().at(1));
			}
		}

		IdAgentImpl::IdAgentImpl(MasterWallet *parentWallet) :
				_parentWallet(parentWallet) {

		}

		IdAgentImpl::~IdAgentImpl() {

		}

		std::string
		IdAgentImpl::DeriveIdAndKeyForPurpose(uint32_t purpose, uint32_t index, const std::string &payPassword) {
			ParamChecker::checkPassword(payPassword);

			if (purpose == 44)
				throw std::invalid_argument("Can not use reserved purpose.");

			IdPath path(purpose, index);
			std::string existedId;
			if (findIdByPath(path, existedId)) {
				return existedId;
			}

			UInt512 seed = _parentWallet->deriveSeed(payPassword);

			BRKey *privkey = new BRKey;
			UInt256 chainCode;
			Key::deriveKeyAndChain(privkey, chainCode, &seed, sizeof(seed), 2, purpose, index);
			Key wrappedKey(privkey);
			std::string id = wrappedKey.keyToAddress(ELA_IDCHAIN);

			_info.Ids[id] = path;
			return id;
		}

		bool IdAgentImpl::IsIdValid(const std::string &id) {
			return Address::isValidIdAddress(id);
		}

		std::string IdAgentImpl::Sign(const std::string &id, const std::string &message, const std::string &password) {
			KeyPtr key = generateKey(id, password);

			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());

			CMBlock signedData = key->sign(md);

			char *data = new char[signedData.GetSize()];
			memcpy(data, signedData, signedData.GetSize());
			std::string singedMsg(data, signedData.GetSize());
			return singedMsg;
		}

		std::vector<std::string> IdAgentImpl::GetAllIds() const {
			std::vector<std::string> result;
			std::for_each(_info.Ids.begin(), _info.Ids.end(), [&result](const IdAgentInfo::IdMap::value_type &item) {
				result.push_back(item.first);
			});
			return result;
		}

		KeyPtr IdAgentImpl::generateKey(const std::string &id, const std::string &password) {
			if(_info.Ids.find(id) != _info.Ids.end()) {
				throw std::logic_error("Unknown id.");
			}
			IdPath path = _info.Ids[id];

			UInt512 seed = _parentWallet->deriveSeed(password);
			BRKey *privkey = new BRKey;
			UInt256 chainCode;
			Key::deriveKeyAndChain(privkey, chainCode, &seed, sizeof(seed), 2, path.Purpose, path.Index);
			return KeyPtr(new Key(privkey));
		}

		bool IdAgentImpl::findIdByPath(const IdPath &path, std::string &id) {
			for (IdAgentInfo::IdMap::iterator it = _info.Ids.begin(); it != _info.Ids.end(); it++) {
				if(it->second == path) {
					id = it->first;
					return true;
				}
			}
			return false;
		}

		std::string IdAgentImpl::GenerateRedeemScript(const std::string &id, const std::string &password) {
			KeyPtr key = generateKey(id, password);
			return key->keyToRedeemScript(ELA_IDCHAIN);
		}

	}
}