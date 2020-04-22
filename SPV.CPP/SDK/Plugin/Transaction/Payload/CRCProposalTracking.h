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
#ifndef __ELASTOS_SDK_CRCPROPOSALTRACKING_H__
#define __ELASTOS_SDK_CRCPROPOSALTRACKING_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

#define CRCProposalTrackingDefaultVersion 0

		class CRCProposalTracking : public IPayload {
		public:
			enum Type {
				common = 0x0000,
				progress = 0x01,
				rejected = 0x02,
				terminated = 0x03,
				changeOwner = 0x04,
				finalized = 0x05,
				unknowTrackingType
			};

			CRCProposalTracking();

			~CRCProposalTracking();

			void SetProposalHash(const uint256 &proposalHash);

			const uint256 &GetProposalHash() const;

			void SetMessageHash(const uint256 &messageHash);

			const uint256 &GetMessageHash() const;

			void SetStage(uint8_t stage);

			uint8_t GetStage() const;

			void SetOwnerPubKey(const bytes_t &ownerPubKey);

			const bytes_t &GetOwnerPubKey() const;

			void SetNewOwnerPubKey(const bytes_t &newOwnerPubKey);

			const bytes_t &GetNewOwnerPubKey() const;

			void SetOwnerSign(const bytes_t &signature);

			const bytes_t &GetOwnerSign() const;

			void SetNewOwnerSign(const bytes_t &signature);

			const bytes_t &GetNewOwnerSign() const;

			void SetType(CRCProposalTracking::Type type);

			CRCProposalTracking::Type GetType() const;

			void SetSecretaryGeneralOpinionHash(const uint256 &hash);

			const uint256 &GetSecretaryGeneralOpinionHash()  const;

			void SetSecretaryGeneralSignature(const bytes_t &signature);

			const bytes_t &GetSecretaryGeneralSignature() const;

			const uint256 &DigestOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestNewOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestSecretaryUnsigned(uint8_t version) const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeOwnerUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeNewOwnerUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeNewOwnerUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeSecretaryUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeSecretaryUnsigned(const ByteStream &stream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);


			nlohmann::json ToJsonOwnerUnsigned(uint8_t version) const;

			void FromJsonOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonNewOwnerUnsigned(uint8_t version) const;

			void FromJsonNewOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonSecretaryUnsigned(uint8_t version) const;

			void FromJsonSecretaryUnsigned(const nlohmann::json &j, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			bool IsValidOwnerUnsigned(uint8_t version) const;

			bool IsValidNewOwnerUnsigned(uint8_t version) const;

			bool IsValidSecretaryUnsigned(uint8_t version) const;

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposalTracking &operator=(const CRCProposalTracking &payload);

		private:
			mutable uint256 _digestOwnerUnsigned, _digestNewOwnerUnsigned, _digestSecretaryUnsigned;

		private:
			uint256 _proposalHash;
			uint256 _messageHash;
			uint8_t _stage;
			bytes_t _ownerPubKey;
			bytes_t _newOwnerPubKey;
			bytes_t _ownerSign;
			bytes_t _newOwnerSign;
			CRCProposalTracking::Type _type;
			uint256 _secretaryGeneralOpinionHash;
			bytes_t _secretaryGeneralSignature;
		};

	}
}


#endif //__ELASTOS_SDK_CRCPROPOSALTRACKING_H__
