// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CRCPROPOSALTRACKING_H__
#define __ELASTOS_SDK_CRCPROPOSALTRACKING_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {
		class CRCProposalTracking : public IPayload {
		public:
			enum CRCProposalTrackingType {
				common = 0x00,
				progress = 0x01,
				progressReject = 0x02,
				terminated = 0x03,
				proposalLeader = 0x04,
				appropriation = 0x05,
				maxType
			};

			CRCProposalTracking();

			~CRCProposalTracking();

			void SetType(CRCProposalTrackingType type);

			CRCProposalTrackingType GetType() const;

			void SetProposalHash(const uint256 &proposalHash);

			const uint256 &GetProposalHash() const;

			void SetDocumentHash(const uint256 &documentHash);

			const uint256 &GetDocumentHash() const;

			void SetStage(uint8_t stage);

			uint8_t GetStage() const;

			void SetAppropriation(uint64_t appropriation);

			uint64_t GetAppropriation() const;

			void SetLeaderPubKey(const bytes_t &leaderPubKey);

			const bytes_t &GetLeaderPubKey() const;

			void SetNewLeaderPubKey(const bytes_t &newLeaderPubKey);

			const bytes_t &GetNewLeaderPubKey() const;

			void SetLeaderSign(const bytes_t &signature);

			const bytes_t &GetLeaderSign() const;

			void SetNewLeaderSign(const bytes_t &signature);

			const bytes_t &GetNewLeaderSign() const;

			void SetSecretaryGeneralSign(const bytes_t &signature);

			const bytes_t &GetSecretaryGeneralSign() const;

		public:
			virtual size_t EstimateSize(uint8_t version) const;

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CRCProposalTracking &operator=(const CRCProposalTracking &payload);

		private:
			CRCProposalTrackingType _type;
			uint256 _proposalHash;
			uint256 _documentHash;
			uint8_t _stage;
			uint64_t _appropriation;
			bytes_t _leaderPubKey;
			bytes_t _newLeaderPubKey;
			bytes_t _leaderSign;
			bytes_t _newLeaderSign;
			bytes_t _secretaryGeneralSign;
		};
	}
}


#endif //__ELASTOS_SDK_CRCPROPOSALTRACKING_H__
