// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H
#define __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H

#include <SDK/Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

namespace Elastos {
	namespace ElaWallet {


		class PayloadVote : public IOutputPayload {
		public:
			enum Type {
				Delegate,
				CRC,
				Max,
			};

			struct VoteContent {
				VoteContent() : type(Type::Delegate) {
				}

				VoteContent(Type t, const std::vector<CMBlock> &c) : type(t), candidates(c) {
				}

				Type type;
				std::vector<CMBlock> candidates;
			};

		public:
			PayloadVote();

			PayloadVote(const std::vector<VoteContent> &voteContents);

			PayloadVote(const PayloadVote &payload);

			~PayloadVote();

			void SetVoteContent(const std::vector<VoteContent> &voteContent);

			const std::vector<VoteContent> &GetVoteContent() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

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
