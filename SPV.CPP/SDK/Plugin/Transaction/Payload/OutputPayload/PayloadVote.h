// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H
#define __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H

#include <SDK/Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

#define VOTE_PRODUCER_CR_VERSION  0x01

namespace Elastos {
	namespace ElaWallet {

		class CandidateVotes {
		public:
			CandidateVotes();

			CandidateVotes(const bytes_t &candidate, uint64_t votes = 0);

			~CandidateVotes();
		public:
			const bytes_t & GetCandidate() const;

			uint64_t GetVotes() const;

			void Serialize(ByteStream &ostream, uint8_t version) const;

			bool Deserialize(const ByteStream &istream, uint8_t version);

			nlohmann::json ToJson(uint8_t version) const;

			void FromJson(const nlohmann::json &j, uint8_t version);
		private:
			bytes_t  _candidate;
			uint64_t _votes;
		};

		class VoteContent {
		public:
			enum Type {
				Delegate,
				CRC,
				Max,
			};

			VoteContent();

			VoteContent(Type t, const std::vector<CandidateVotes> &c);

			~VoteContent();
		public:
			void AddCandidate(const CandidateVotes &candidateVotes);

			const Type &GetType() const;

			const std::vector<CandidateVotes> &GetCandidates() const;

			void Serialize(ByteStream &ostream, uint8_t version) const;

			bool Deserialize(const ByteStream &istream, uint8_t version);

			nlohmann::json ToJson(uint8_t version) const;

			void FromJson(const nlohmann::json &j, uint8_t version);
		private:
			Type _type;
			std::vector<CandidateVotes> _candidates;
		};

		class PayloadVote : public IOutputPayload {
		public:
			PayloadVote(uint8_t version = 0);

			PayloadVote(const std::vector<VoteContent> &voteContents, uint8_t version = 0);

			PayloadVote(const PayloadVote &payload);

			~PayloadVote();

			void SetVoteContent(const std::vector<VoteContent> &voteContent);

			const std::vector<VoteContent> &GetVoteContent() const;

			virtual size_t EstimateSize() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual IOutputPayload &operator=(const IOutputPayload &payload);

			virtual PayloadVote &operator=(const PayloadVote &payload);

		private:
			uint8_t _version;
			std::vector<VoteContent> _content;
		};
	}
}

#endif //__ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H
