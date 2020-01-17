// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CRCPROPOSAL_H__
#define __ELASTOS_SDK_CRCPROPOSAL_H__

#include "IPayload.h"
#include <Common/BigInt.h>

namespace Elastos {
	namespace ElaWallet {
		class Budget {
		public:
			enum BudgetType {
				imprest = 0x00,
				normalPayment = 0x01,
				finalPayment = 0x02,
				maxType
			};
			Budget();

			Budget(BudgetType type, uint8_t stage, BigInt amount);

			~Budget();

			BudgetType GetType() const;

			uint8_t GetStage() const;

			BigInt GetAmount() const;

			void Serialize(ByteStream &ostream, uint8_t version) const;

			bool Deserialize(const ByteStream &istream, uint8_t version);

			nlohmann::json ToJson(uint8_t version) const;

			void FromJson(const nlohmann::json &j, uint8_t version);

		private:
			BudgetType _type;
			uint8_t _stage;
			BigInt _amount;
		};

		class CRCProposal : public IPayload {
		public:
			enum CRCProposalType {
				normal = 0x0000,
				elip = 0x0100,
				flowElip = 0x0101,
				infoElip = 0x0102,
				mainChainUpgradeCode = 0x0200,
				sideChainUpgradeCode = 0x0300,
				registerSideChain = 0x0301,
				secretaryGeneral = 0x0400,
				changeSponsor = 0x0401,
				closeProposal = 0x0402,
				dappConsensus = 0x0500,
				maxType
			};

			CRCProposal();

			~CRCProposal();

			void SetTpye(CRCProposalType type);

			CRCProposalType GetType() const;

			void SetCategoryData(const std::string &categoryData);

			const std::string &GetCategoryData() const;

			void SetSponsorPublicKey(const bytes_t &publicKey);

			const bytes_t &GetSponsorPublicKey() const;

			void SetCRSponsorDID(const uint168 &crSponsorDID);

			const uint168 &GetCRSponsorDID() const;

			void SetDraftHash(const uint256 &draftHash);

			const uint256 &GetDraftHash() const;

			void SetBudgets(const std::vector <Budget> &budgets);

			const std::vector <Budget> &GetBudgets() const;

			void SetRecipient(const uint168 &recipient);

			const uint168 &GetRecipient() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			void SetCRSignature(const bytes_t &signature);

			const bytes_t &GetCRSignature() const;

			void SetCROpinionHash(const uint256 &hash);

			const uint256 &GetCROpinionHash() const;

			uint256 Hash() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			void SerializeSponsorSigned(ByteStream &ostream, uint8_t version);

			bool DeserializeSponsorSigned(const ByteStream &istream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposal &operator=(const CRCProposal &payload);

		private:
			CRCProposalType _type;
			std::string _categoryData;
			bytes_t _sponsorPublicKey;
			uint168 _crSponsorDID;
			uint256 _draftHash;
			std::vector <Budget> _budgets;
			uint168 _recipient;
			bytes_t _signature;
			uint256 _crOpinionHash;
			bytes_t _crSignature;
		};
	}
}
#endif //__ELASTOS_SDK_CRCPROPOSAL_H__
