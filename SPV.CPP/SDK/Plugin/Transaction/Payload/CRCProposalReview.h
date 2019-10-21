// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CRCPROPOSALREVIEW_H__
#define __ELASTOS_SDK_CRCPROPOSALREVIEW_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class CRCProposalReview : public IPayload {
		public:
			enum VoteResult {
				approve = 0x00,
				reject = 0x01,
				abstain = 0x02
			};

			CRCProposalReview();

			~CRCProposalReview();

			void SetProposalHash(const uint256 &hash);

			const uint256 &GetProposalHash() const;

			void SetResult(VoteResult result);

			VoteResult GetResult() const;

			void SetCRDID(const uint168 &crDID);

			const uint168 &GetCRDID() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposalReview &operator=(const CRCProposalReview &payload);

		private:
			uint256 _proposalHash;
			VoteResult _result;
			uint168 _crDID;
			bytes_t _signature;
		};
	}
}
#endif //__ELASTOS_SDK_CRCPROPOSALREVIEW_H__