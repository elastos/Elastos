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

			bool operator==(const Budget &budget) const;

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
				flowElip = 0x0101,
				infoElip = 0x0102,
				mainChainUpgradeCode = 0x0200,
				sideChainUpgradeCode = 0x0300,
				registerSideChain = 0x0301,
				secretaryGeneralElection = 0x0400,
				changeProposalOwner = 0x0401,
				terminateProposal = 0x0402,
				dappConsensus = 0x0500,
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

			void SetDraftHash(const uint256 &draftHash);

			const uint256 &GetDraftHash() const;

			void SetBudgets(const std::vector<Budget> &budgets);

			const std::vector<Budget> &GetBudgets() const;

			void SetRecipient(const Address &recipient);

			const Address &GetRecipient() const;

			void SetTargetProposalHash(const uint256 &hash);

			const uint256 &GetTargetProposalHash() const;

			void SetNewRecipient(const Address &recipient);

			const Address &GetNewRecipient() const;

			void SetNewOwnerPublicKey(const bytes_t &pubkey);

			const bytes_t &GetNewOwnerPublicKey() const;

			void SetSecretaryPublicKey(const bytes_t &pubkey);

			const bytes_t GetSecretaryPublicKey() const;

			void SetSecretaryDID(const Address &did);

			const Address &GetSecretaryDID() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			void SetNewOwnerSignature(const bytes_t &sign);

			const bytes_t &GetNewOwnerSignature() const;

			void SetSecretarySignature(const bytes_t &sign);

			const bytes_t &GetSecretarySignature() const;

			void SetCRCouncilMemberDID(const Address &crSponsorDID);

			const Address &GetCRCouncilMemberDID() const;

			void SetCRCouncilMemberSignature(const bytes_t &signature);

			const bytes_t &GetCRCouncilMemberSignature() const;

		public:
			// normal or elip
			void SerializeOwnerUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeOwnerUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeCRCouncilMemberUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeCRCouncilMemberUnsigned(const ByteStream &istream, uint8_t version);

			void SerializeNormalOrELIP(ByteStream &stream, uint8_t version) const;

			bool DeserializeNormalOrELIP(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonNormalOwnerUnsigned(uint8_t version) const;

			void FromJsonNormalOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonNormalCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonNormalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidNormalOwnerUnsigned(uint8_t version) const;

			bool IsValidNormalCRCouncilMemberUnsigned(uint8_t version) const;

			const uint256 &DigestNormalOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestNormalCRCouncilMemberUnsigned(uint8_t version) const;

			// change owner
			void SerializeChangeOwnerUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeChangeOwnerUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeChangeOwnerCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeChangeOwnerCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeChangeOwner(ByteStream &stream, uint8_t version) const;

			bool DeserializeChangeOwner(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonChangeOwnerUnsigned(uint8_t version) const;

			void FromJsonChangeOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonChangeOwnerCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidChangeOwnerUnsigned(uint8_t version) const;

			bool IsValidChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const;

			const uint256 &DigestChangeOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const;

			// terminate proposal
			void SerializeTerminateProposalUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeTerminateProposalUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeTerminateProposalCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeTerminateProposalCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeTerminateProposal(ByteStream &stream, uint8_t version) const;

			bool DeserializeTerminateProposal(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonTerminateProposalOwnerUnsigned(uint8_t version) const;

			void FromJsonTerminateProposalOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonTerminateProposalCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidTerminateProposalOwnerUnsigned(uint8_t version) const;

			bool IsValidTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const;

			const uint256 &DigestTerminateProposalOwnerUnsigned(uint8_t version) const;

			const uint256 &DigestTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const;

			// secretary election
			void SerializeSecretaryElectionUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeSecretaryElectionUnsigned(const ByteStream &stream, uint8_t verion);

			void SerializeSecretaryElectionCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeSecretaryElectionCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeSecretaryElection(ByteStream &stream, uint8_t version) const;

			bool DeserializeSecretaryElection(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonSecretaryElectionUnsigned(uint8_t version) const;

			void FromJsonSecretaryElectionUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonSecretaryElectionCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidSecretaryElectionUnsigned(uint8_t version) const;

			bool IsValidSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const;

			const uint256 &DigestSecretaryElectionUnsigned(uint8_t version) const;

			const uint256 &DigestSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const;

			// override interface
			size_t EstimateSize(uint8_t version) const override;

			// top serialize or deserialize
			void Serialize(ByteStream &stream, uint8_t version) const override;

			bool Deserialize(const ByteStream &stream, uint8_t version) override;

			nlohmann::json ToJson(uint8_t version) const override;

			void FromJson(const nlohmann::json &j, uint8_t version) override;

			bool IsValid(uint8_t version) const override;

			IPayload &operator=(const IPayload &payload) override;

			CRCProposal &operator=(const CRCProposal &payload);

			bool operator==(const IPayload &payload) const;

		private:
			// normal & elip
			mutable uint256 _digestOwnerUnsigned;
			mutable uint256 _digestCRCouncilMemberUnsigned;

			// secretary election
			mutable uint256 _digestSecretaryElectionUnsigned;
			mutable uint256 _digestSecretaryElectionCRCouncilMemberUnsigned;

			// change owner
			mutable uint256 _digestChangeOwnerUnsigned;
			mutable uint256 _digestChangeOwnerCRCouncilMemberUnsigned;

			// terminate proposal
			mutable uint256 _digestTerminateProposalOwnerUnsigned;
			mutable uint256 _digestTerminateProposalCRCouncilMemberUnsigned;

		private:
			CRCProposal::Type _type;
			std::string _categoryData;
			bytes_t _ownerPublicKey;
			uint256 _draftHash;
			std::vector <Budget> _budgets;
			Address _recipient;
			uint256 _targetProposalHash;
			Address _newRecipient;
			bytes_t _newOwnerPublicKey;
			bytes_t _secretaryPublicKey;
			Address _secretaryDID;
			bytes_t _signature;
			bytes_t _newOwnerSignature;
			bytes_t _secretarySignature;

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
