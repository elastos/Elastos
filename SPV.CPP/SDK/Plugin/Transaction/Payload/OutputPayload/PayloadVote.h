// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H
#define __ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H

#include <SDK/Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {
		class PayloadVote :
			public IPayload {
		public:
			enum Type {
				Delegate,
				CRC,
			};

		public:
			PayloadVote();

			PayloadVote(const Type &type, const std::vector<CMBlock> &candidates);

			PayloadVote(const PayloadVote &payload);

			~PayloadVote();

			void SetVoteType(const Type &type);

			const Type &GetVoteType() const;

			void SetCandidates(const std::vector<CMBlock> &candidates);

			const std::vector<CMBlock> &GetCandidates() const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			virtual IPayload &operator=(const IPayload &payload);

			virtual PayloadVote &operator=(const PayloadVote &payload);

		private:
			Type _voteType;
			std::vector<CMBlock> _candidates;
		};
	}
}

#endif //__ELASTOS_SDK_OUTPUT_PAYLOADVOTE_H
