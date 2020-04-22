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
#ifndef __ELASTOS_SDK_CRCPROPOSAL_H__
#define __ELASTOS_SDK_CRCPROPOSAL_H__

#include "IPayload.h"
#include <Common/BigInt.h>
#include <Common/JsonSerializer.h>
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {

#define CRCProposalDefaultVersion 0
		class Budget : public JsonSerializer {
		public:
			enum Type {
				imprest = 0x00,
				normalPayment = 0x01,
				finalPayment = 0x02,
				maxType
			};
			Budget();

			Budget(Budget::Type type, uint8_t stage, const BigInt &amount);

			~Budget();

			Budget::Type GetType() const;

			uint8_t GetStage() const;

			BigInt GetAmount() const;

			void Serialize(ByteStream &ostream) const;

			bool Deserialize(const ByteStream &istream);

			bool IsValid() const;

			nlohmann::json ToJson() const override;

			void FromJson(const nlohmann::json &j) override;

		private:
			Budget::Type _type;
			uint8_t _stage;
			BigInt _amount;
		};

		class CRCProposal : public IPayload {
		public:
			enum Type {
				normal = 0x0000,
				elip = 0x0100,
//				flowElip = 0x0101,
//				infoElip = 0x0102,
//				mainChainUpgradeCode = 0x0200,
//				sideChainUpgradeCode = 0x0300,
//				registerSideChain = 0x0301,
//				secretaryGeneral = 0x0400,
//				changeSponsor = 0x0401,
//				closeProposal = 0x0402,
//				dappConsensus = 0x0500,
				maxType
			};

			CRCProposal();

			~CRCProposal();

			void SetTpye(CRCProposal::Type type);

			CRCProposal::Type GetType() const;

			void SetCategoryData(const std::string &categoryData);

			const std::string &GetCategoryData() const;

			void SetOwnerPublicKey(const bytes_t &publicKey);

			const bytes_t &GetOwnerPublicKey() const;

			void SetCRCouncilMemberDID(const Address &crSponsorDID);

			const Address &GetCRCouncilMemberDID() const;

			void SetDraftHash(const uint256 &draftHash);

			const uint256 &GetDraftHash() const;

			void SetBudgets(const std::vector<Budget> &budgets);

			const std::vector<Budget> &GetBudgets() const;

			void SetRecipient(const Address &recipient);

			const Address &GetRecipient() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			void SetCRCouncilMemberSignature(const bytes_t &signature);

			const bytes_t &GetCRCouncilMemberSignature() const;

			const uint256 &DigestOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestCRCouncilMemberUnsigned(uint8_t version) const;

		public:
			size_t EstimateSize(uint8_t version) const override;

			void SerializeOwnerUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeOwnerUnsigned(const ByteStream &istream, uint8_t version);

			void SerializeCRCouncilMemberUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeCRCouncilMemberUnsigned(const ByteStream &istream, uint8_t version);

			void Serialize(ByteStream &ostream, uint8_t version) const override;

			bool Deserialize(const ByteStream &istream, uint8_t version) override;

			nlohmann::json ToJsonOwnerUnsigned(uint8_t version) const;

			void FromJsonOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJson(uint8_t version) const override;

			void FromJson(const nlohmann::json &j, uint8_t version) override;

			bool IsValidOwnerUnsigned(uint8_t version) const;

			bool IsValidCRCouncilMemberUnsigned(uint8_t version) const;

			bool IsValid(uint8_t version) const override;

			IPayload &operator=(const IPayload &payload) override;

			CRCProposal &operator=(const CRCProposal &payload);

		private:
			mutable uint256 _digestOwnerUnsigned;
			mutable uint256 _digestCRCouncilMemberUnsigned;

		private:
			CRCProposal::Type _type;
			std::string _categoryData;
			bytes_t _ownerPublicKey;
			uint256 _draftHash;
			std::vector <Budget> _budgets;
			Address _recipient;
			bytes_t _signature;

			// cr council member did
			Address _crCouncilMemberDID;
			bytes_t _crCouncilMemberSignature;
		};
	}
}

namespace nlohmann {
	template<>
	struct adl_serializer<Elastos::ElaWallet::Budget> {
		static void to_json(json &j, const Elastos::ElaWallet::Budget &budget) {
			j = budget.ToJson();
		}

		static void from_json(const json &j, Elastos::ElaWallet::Budget &budget) {
			budget.FromJson(j);
		}
	};
}

#endif //__ELASTOS_SDK_CRCPROPOSAL_H__
