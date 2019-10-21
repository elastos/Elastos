// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CRCPROPOSAL_H
#define __ELASTOS_SDK_CRCPROPOSAL_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class CRCProposal : public IPayload {
		public:
			enum CRCProposalType {
				normal = 0x00,
				code = 0x01,
				sideChain = 0x02,
				changeSponsor = 0x03,
				closeProposal = 0x04,
				secretaCRCProposalryGeneral = 0x05,
				maxType
			};
			CRCProposal();

			~CRCProposal();

			void SetTpye(CRCProposalType type);

			CRCProposalType GetType() const;

			void SetSponsorPublicKey(const bytes_t &publicKey);

			const bytes_t &GetSponsorPublicKey() const;

			void SetCRSponsorDID(const uint168 &crSponsorDID);

			const uint168 &GetCRSponsorDID() const;

			void SetDraftHash(const uint256 &draftHash);

			const uint256 &GetDraftHash() const;

			void SetBudgets(const std::vector<uint64_t> &budgets);

			const std::vector<uint64_t> &GetBudgets() const;

			void SetRecipient(const uint168 &recipient);

			const uint168 &GetRecipient() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			void SetCRSignature(const bytes_t &signature);

			const bytes_t &GetCRSignature() const;

			uint256 Hash() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposal &operator=(const CRCProposal &payload);

		private:
			CRCProposalType _type;
			bytes_t _sponsorPublicKey;
			uint168 _crSponsorDID;
			uint256 _draftHash;
			std::vector<uint64_t> _budgets;
			uint168 _recipient;
			bytes_t _signature;
			bytes_t _crSignature;
		};
	}
}
#endif //__ELASTOS_SDK_CRCPROPOSAL_H
