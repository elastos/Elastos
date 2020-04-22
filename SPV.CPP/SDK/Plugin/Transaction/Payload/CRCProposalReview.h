/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_CRCPROPOSALREVIEW_H__
#define __ELASTOS_SDK_CRCPROPOSALREVIEW_H__

#include "IPayload.h"
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {

#define CRCProposalReviewDefaultVersion 0
		class CRCProposalReview : public IPayload {
		public:
			enum VoteResult {
				approve = 0x00,
				reject = 0x01,
				abstain = 0x02,
				unknownVoteResult
			};

			CRCProposalReview();

			~CRCProposalReview();

			void SetProposalHash(const uint256 &hash);

			const uint256 &GetProposalHash() const;

			void SetVoteResult(VoteResult voteResult);

			VoteResult GetVoteResult() const;

			void SetOpinionHash(const uint256 &hash);

			const uint256 &GetOpinionHash() const;

			void SetDID(const Address &DID);

			const Address &GetDID() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			const uint256 &DigestUnsigned(uint8_t version) const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			nlohmann::json ToJsonUnsigned(uint8_t version) const;

			void FromJsonUnsigned(const nlohmann::json &j, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			bool IsValidUnsigned(uint8_t version) const;

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposalReview &operator=(const CRCProposalReview &payload);

		private:
			mutable uint256 _digest;

		private:
			uint256 _proposalHash;
			VoteResult _voteResult;
			uint256 _opinionHash;
			Address _did;
			bytes_t _signature;
		};
	}
}
#endif //__ELASTOS_SDK_CRCPROPOSALREVIEW_H__