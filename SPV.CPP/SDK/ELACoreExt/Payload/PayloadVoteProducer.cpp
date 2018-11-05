// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadVoteProducer.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadVoteProducer::PayloadVoteProducer() {

		}

		PayloadVoteProducer::~PayloadVoteProducer() {

		}

		const std::string &PayloadVoteProducer::GetVoter() const {
			return _voter;
		}

		void PayloadVoteProducer::SetVoter(const std::string &voter) {
			_voter = voter;
		}

		uint64_t PayloadVoteProducer::GetStake() const {
			return _stake;
		}

		void PayloadVoteProducer::SetStake(uint64_t stake) {
			_stake = stake;
		}

		const std::vector<std::string> &PayloadVoteProducer::GetPublicKeys() const {
			return _publicKeys;
		}

		void PayloadVoteProducer::SetPublicKeys(const std::vector<std::string> &keys) {
			_publicKeys = keys;
		}

		void PayloadVoteProducer::Serialize(ByteStream &ostream) const {
			ostream.writeVarString(_voter);
			ostream.writeUint64(_stake);

			ostream.writeVarUint(_publicKeys.size());
			std::for_each(_publicKeys.begin(), _publicKeys.end(), [&ostream](const std::string &key) {
				ostream.writeVarString(key);
			});
		}

		bool PayloadVoteProducer::Deserialize(ByteStream &istream) {
			if (!istream.readVarString(_voter))
				return false;
			if (!istream.readUint64(_stake))
				return false;

			uint64_t size;
			if(!istream.readVarUint(size))
				return false;
			_publicKeys.clear();
			std::string temp;
			for (uint64_t i = 0; i < size; ++i) {
				if(!istream.readVarString(temp))
					return false;
				_publicKeys.push_back(temp);
			}
			return true;
		}

		nlohmann::json PayloadVoteProducer::toJson() const {
			nlohmann::json j;
			j["Voter"] = _voter;
			j["Stake"] = _stake;
			j["PublicKeys"] = _publicKeys;

			return j;
		}

		void PayloadVoteProducer::fromJson(const nlohmann::json &j) {
			_voter = j["Voter"].get<std::string>();
			_stake = j["Stake"].get<uint64_t>();
			std::vector<nlohmann::json> keys = j["PublicKeys"];
			_publicKeys.clear();
			std::for_each(keys.begin(), keys.end(), [this](const nlohmann::json &keyJson) {
				_publicKeys.push_back(keyJson.get<std::string>());
			});
		}
	}
}
