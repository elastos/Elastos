// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadVote.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>

namespace Elastos {
	namespace ElaWallet {
		PayloadVote::PayloadVote() :
			_voteType(PayloadVote::Type::Delegate) {
		}

		PayloadVote::PayloadVote(const PayloadVote::Type &type, const std::vector <CMBlock> &candidates) :
			_voteType(type),
			_candidates(candidates) {
		}

		PayloadVote::PayloadVote(const PayloadVote &payload){
			operator=(payload);
		}

		PayloadVote::~PayloadVote() {

		}

		void PayloadVote::SetVoteType(const PayloadVote::Type &type) {
			_voteType = type;
		}

		const PayloadVote::Type &PayloadVote::GetVoteType() const {
			return _voteType;
		}

		void PayloadVote::SetCandidates(const std::vector<CMBlock> &candidates) {
			_candidates = candidates;
		}

		const std::vector<CMBlock> &PayloadVote::GetCandidates() const {
			return _candidates;
		}

		void PayloadVote::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeUint8(_voteType);
			ostream.writeVarUint(_candidates.size());
			for (size_t i = 0; i < _candidates.size(); ++i) {
				ostream.writeVarBytes(_candidates[i]);
			}
		}

		bool PayloadVote::Deserialize(ByteStream &istream, uint8_t version) {
			uint8_t voteType = 0;
			if (!istream.readUint8(voteType)) {
				Log::error("payload vote deserialize type error");
				return false;
			}
			_voteType = static_cast<Type>(voteType);

			size_t candidateCount = 0;
			if (!istream.readVarUint(candidateCount)) {
				Log::error("payload vote deserialize candidate count error");
				return false;
			}

			_candidates.clear();
			for (size_t i = 0; i < candidateCount; ++i) {
				CMBlock candidate;
				if (!istream.readVarBytes(candidate)) {
					Log::error("payload vote deserialize candidate[{}] error", i);
				}
				_candidates.push_back(candidate);
			}

			return true;
		}

		nlohmann::json PayloadVote::toJson(uint8_t version) const {
			nlohmann::json j;
			j["VoteType"] = _voteType;
			std::vector<std::string> candidates;
			for (size_t i = 0; i < _candidates.size(); ++i) {
				candidates.push_back(Utils::encodeHex(_candidates[i]));
			}
			j["Candidates"] = candidates;

			return j;
		}

		void PayloadVote::fromJson(const nlohmann::json &j, uint8_t version) {
			_voteType = j["VoteType"];
			std::vector<std::string> candidates;
			candidates = j["Candidates"].get<std::vector<std::string>>();
			_candidates.clear();
			for (size_t i = 0; i < candidates.size(); ++i) {
				CMBlock candidate = Utils::decodeHex(candidates[i]);
				_candidates.push_back(candidate);
			}
		}

		IPayload &PayloadVote::operator=(const IPayload &payload) {
			try {
				const PayloadVote &payloadVote = dynamic_cast<const PayloadVote &>(payload);
				operator=(payloadVote);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadVote");
			}

			return *this;
		}

		PayloadVote& PayloadVote::operator=(const PayloadVote &payload) {
			_voteType = payload._voteType;
			_candidates.resize(payload._candidates.size());
			for (size_t i = 0; i < _candidates.size(); ++i) {
				_candidates[i].Memcpy(payload._candidates[i]);
			}

			return *this;
		}

	}
}
