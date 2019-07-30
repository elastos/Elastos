// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadVote.h"
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {
		CandidateVotes::CandidateVotes() : _votes(0) {

		}

		CandidateVotes::CandidateVotes(const bytes_t &candidate, uint64_t votes) :
				_candidate(candidate), _votes(votes) {

		}

		CandidateVotes::~CandidateVotes() {

		}

		const bytes_t &CandidateVotes::GetCandidate() const {
			return _candidate;
		}

		uint64_t CandidateVotes::GetVotes() const {
			return _votes;
		}

		void CandidateVotes::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_candidate);

			if (version >= VOTE_PRODUCER_CR_VERSION) {
				ostream.WriteUint64(_votes);
			}
		}

		bool CandidateVotes::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarBytes(_candidate)) {
				return false;
			}

			if (version >= VOTE_PRODUCER_CR_VERSION && !istream.ReadUint64(_votes)) {
				return false;
			}

			return true;
		}

		nlohmann::json CandidateVotes::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Candidate"] = _candidate.getHex();
			if (version >= VOTE_PRODUCER_CR_VERSION) {
				j["Votes"] = _votes;
			}
			return j;
		}

		void CandidateVotes::FromJson(const nlohmann::json &j, uint8_t version) {
			_candidate.setHex(j["Candidate"].get<std::string>());
			if (version >= VOTE_PRODUCER_CR_VERSION) {
				_votes = j["Votes"].get<uint64_t>();
			}
		}

		VoteContent::VoteContent() : _type(Delegate) {

		}

		VoteContent::VoteContent(Type t, const std::vector<CandidateVotes> &c) : _type(Delegate), _candidates(c) {

		}

		VoteContent::~VoteContent() {

		}

		void VoteContent::AddCandidate(const CandidateVotes &candidateVotes) {
			_candidates.push_back(candidateVotes);
		}

		const VoteContent::Type &VoteContent::GetType() const {
			return _type;
		}

		const std::vector<CandidateVotes> &VoteContent::GetCandidates() const {
			return _candidates;
		}

		void VoteContent::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.WriteUint8(_type);

			ostream.WriteVarUint(_candidates.size());
			for (size_t i = 0; i < _candidates.size(); ++i) {
				_candidates[i].Serialize(ostream, version);
			}
		}

		bool VoteContent::Deserialize(const ByteStream &istream, uint8_t version) {
			uint8_t type = 0;
			if (!istream.ReadUint8(type)) {
				Log::error("VoteContent deserialize type error");
			}
			_type = Type(type);

			uint64_t size = 0;
			if (!istream.ReadVarUint(size)) {
				Log::error("VoteContent deserialize candidates count error");
				return false;
			}

			_candidates.resize(size);
			for (size_t i = 0; i < size; ++i) {
				if (!_candidates[i].Deserialize(istream, version)) {
					Log::error("VoteContent deserialize candidates error");
					return false;
				}
			}

			return true;
		}

		nlohmann::json VoteContent::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Type"] = _type;

			std::vector<nlohmann::json> candidates;
			for (size_t i = 0; i < _candidates.size(); ++i) {
				candidates.push_back(_candidates[i].ToJson(version));
			}
			j["Candidates"] = candidates;

			return j;
		}

		void VoteContent::FromJson(const nlohmann::json &j, uint8_t version) {
			uint8_t type = j["Type"].get<uint8_t>();
			_type = Type(type);

			std::vector<nlohmann::json> candidates = j["Candidates"];
			_candidates.resize(candidates.size());
			for (size_t i = 0; i < candidates.size(); ++i) {
				_candidates[i].FromJson(candidates[i], version);
			}
		}


		PayloadVote::PayloadVote(uint8_t version) :
				_version(version) {
		}

		PayloadVote::PayloadVote(const std::vector<VoteContent> &voteContents, uint8_t version) :
				_content(voteContents),
				_version(version) {
		}

		PayloadVote::PayloadVote(const PayloadVote &payload) {
			operator=(payload);
		}

		PayloadVote::~PayloadVote() {

		}

		void PayloadVote::SetVoteContent(const std::vector<VoteContent> &voteContent) {
			_content = voteContent;
		}

		const std::vector<VoteContent> &PayloadVote::GetVoteContent() const {
			return _content;
		}

		size_t PayloadVote::EstimateSize() const {
			ByteStream stream;
			size_t size = 0;

			size += 1;
			size += stream.WriteVarUint(_content.size());
			for (size_t i = 0; i < _content.size(); ++i) {
				size += 1;
				size += stream.WriteVarUint(_content[i].GetCandidates().size());
				for (size_t j = 0; j < _content[i].GetCandidates().size(); ++j) {
					size += stream.WriteVarUint(_content[i].GetCandidates()[j].GetCandidate().size());
					size += _content[i].GetCandidates()[j].GetCandidate().size();

					if (_version >= VOTE_PRODUCER_CR_VERSION) {
						size += stream.WriteVarUint(_content[i].GetCandidates()[j].GetVotes());
					}
				}
			}

			return size;
		}

		void PayloadVote::Serialize(ByteStream &ostream) const {
			ostream.WriteUint8(_version);

			ostream.WriteVarUint(_content.size());
			for (size_t i = 0; i < _content.size(); ++i) {
				_content[i].Serialize(ostream, _version);
			}
		}

		bool PayloadVote::Deserialize(const ByteStream &istream) {
			if (!istream.ReadUint8(_version)) {
				Log::error("payload vote deserialize version error");
				return false;
			}

			uint64_t contentCount = 0;
			if (!istream.ReadVarUint(contentCount)) {
				Log::error("payload vote deserialize content count error");
				return false;
			}

			_content.resize(contentCount);
			for (size_t i = 0; i < contentCount; ++i) {
				if (!_content[i].Deserialize(istream, _version)) {
					Log::error("payload vote deserialize content error");
					return false;
				}
			}

			return true;
		}

		nlohmann::json PayloadVote::ToJson() const {
			nlohmann::json j;
			j["Version"] = _version;

			std::vector<nlohmann::json> voteContent;
			for (size_t i = 0; i < _content.size(); ++i) {
				voteContent.push_back(_content[i].ToJson(_version));
			}
			j["VoteContent"] = voteContent;

			return j;
		}

		void PayloadVote::FromJson(const nlohmann::json &j) {
			_version = j["Version"];
			std::vector<nlohmann::json> voteContent = j["VoteContent"];
			_content.resize(voteContent.size());

			for (size_t i = 0; i < voteContent.size(); ++i) {
				_content[i].FromJson(voteContent[i], _version);
			}
		}

		IOutputPayload &PayloadVote::operator=(const IOutputPayload &payload) {
			try {
				const PayloadVote &payloadVote = dynamic_cast<const PayloadVote &>(payload);
				operator=(payloadVote);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadVote");
			}

			return *this;
		}

		PayloadVote &PayloadVote::operator=(const PayloadVote &payload) {
			_version = payload._version;
			_content = payload._content;

			return *this;
		}

	}
}
