/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SPVSDK_CRCOUNCILMEMBERCLAIMNODE_H__
#define __ELASTOS_SPVSDK_CRCOUNCILMEMBERCLAIMNODE_H__

#include <WalletCore/Address.h>
#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

#define CRCouncilMemberClaimNodeVersion 0

		class CRCouncilMemberClaimNode : public IPayload {
		public:
			CRCouncilMemberClaimNode();

			~CRCouncilMemberClaimNode();

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &stream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &stream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			CRCouncilMemberClaimNode &operator=(const CRCouncilMemberClaimNode &payload);

			virtual bool Equal(const IPayload &payload, uint8_t version) const;

		public:
			void SerializeUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonUnsigned(uint8_t version) const;

			void FromJsonUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidUnsigned(uint8_t version) const;

			const uint256 &DigestUnsigned(uint8_t version) const;

			void SetNodePublicKey(const bytes_t &pubkey);

			void SetCRCouncilMemberDID(const Address &did);

			void SetCRCouncilMemberSignature(const bytes_t &signature);

		private:
			mutable uint256 _digestUnsigned;

		private:
			bytes_t _nodePublicKey;
			Address _crCouncilMemberDID;
			bytes_t _crCouncilMemberSignature;
		};

	}
}

#endif
