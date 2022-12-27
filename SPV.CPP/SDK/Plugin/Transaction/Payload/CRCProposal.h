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
#define CRCProposalVersion01 0x01

#define JsonKeyType "Type"
#define JsonKeyStage "Stage"
#define JsonKeyAmount "Amount"

#define JsonKeyType "Type"
#define JsonKeyCategoryData "CategoryData"
#define JsonKeyOwnerPublicKey "OwnerPublicKey"
#define JsonKeyDraftHash "DraftHash"
#define JsonKeyDraftData "DraftData"
#define JsonKeyBudgets "Budgets"
#define JsonKeyRecipient "Recipient"
#define JsonKeyTargetProposalHash "TargetProposalHash"
#define JsonKeyNewRecipient "NewRecipient"
#define JsonKeyNewOwnerPublicKey "NewOwnerPublicKey"
#define JsonKeySecretaryPublicKey "SecretaryGeneralPublicKey"
#define JsonKeyReservedCustomIDList "ReservedCustomIDList"
#define JsonKeyReceiverDID "ReceiverDID"
#define JsonKeyReceivedCustomIDList "ReceivedCustomIDList"
#define JsonKeyCustomIDFeeRateInfo "CustomIDFeeRateInfo"
#define JsonKeySecretaryDID "SecretaryGeneralDID"
#define JsonKeySignature "Signature"
#define JsonKeyNewOwnerSignature "NewOwnerSignature"
#define JsonKeySecretarySignature "SecretaryGeneralSignature"
#define JsonKeyCRCouncilMemberDID "CRCouncilMemberDID"
#define JsonKeyCRCouncilMemberSignature "CRCouncilMemberSignature"
#define JsonKeySidechainInfo "SidechainInfo"
#define JsonKeyUpgradeCodeInfo "UpgradeCodeInfo"

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

        class CRCProposal;

        class UpgradeCodeInfo {
        public:
            UpgradeCodeInfo();

            ~UpgradeCodeInfo();

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJson(uint8_t version) const;

            void FromJson(const nlohmann::json &j, uint8_t version);

            bool IsValid(uint8_t version) const;

        private:
            friend class CRCProposal;

            uint32_t _workingHeight;
            std::string _nodeVersion;
            std::string _nodeDownloadUrl;
            uint256 _nodeBinHash;
            bool _force;
        };

        class SideChainInfo {
        public:
            SideChainInfo();

            ~SideChainInfo();

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJson(uint8_t version) const;

            void FromJson(const nlohmann::json &j, uint8_t version);

            bool IsValid(uint8_t version) const;

            bool operator==(const SideChainInfo &info) const;

            SideChainInfo &operator=(const SideChainInfo &info);

        private:
            std::string _sideChainName;
            uint32_t _magicNumber;
            uint256 _genesisHash;
            uint64_t _exchangeRate;
            uint32_t _effectiveHeight;
            std::string _resourcePath;
        };

        class CustomIDFeeRateInfo {
        public:
            CustomIDFeeRateInfo();

            ~CustomIDFeeRateInfo();

            void Serialize(ByteStream &stream, uint8_t version) const;

            bool Deserialize(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJson(uint8_t version) const;

            void FromJson(const nlohmann::json &j, uint8_t version);

            bool operator==(const CustomIDFeeRateInfo &info) const;

            CustomIDFeeRateInfo &operator=(const CustomIDFeeRateInfo &info);

        private:
            // The rate of custom DID fee.
            uint64_t _rateOfCustomIDFee;
            // Effective at the side chain height of EID.
            uint32_t _eIDEffectiveHeight;
        };

		class CRCProposal : public IPayload {
		public:
			enum Type {
				normal = 0x0000,

				elip = 0x0100,
				flowElip = 0x0101,
				infoElip = 0x0102,

				mainChainUpgradeCode = 0x0200,
				didUpdateCode = 0x0201,
				ethUpdateCode = 0x0202,

				secretaryGeneralElection = 0x0400,
				changeProposalOwner = 0x0401,
				terminateProposal = 0x0402,
				registerSideChain = 0x0410,

                reserveCustomID = 0x0500,
                receiveCustomID = 0x0501,
                changeCustomIDFee = 0x0502,
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

			void SetDraftData(const bytes_t &draftData);

			const bytes_t &GetDraftData() const;

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
			void SerializeOwnerUnsigned(ByteStream &stream, uint8_t version) const;

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

			uint256 DigestNormalOwnerUnsigned(uint8_t version) const;

			uint256 DigestNormalCRCouncilMemberUnsigned(uint8_t version) const;

        public:
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

			uint256 DigestChangeOwnerUnsigned(uint8_t version) const;

			uint256 DigestChangeOwnerCRCouncilMemberUnsigned(uint8_t version) const;

        public:
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

			uint256 DigestTerminateProposalOwnerUnsigned(uint8_t version) const;

			uint256 DigestTerminateProposalCRCouncilMemberUnsigned(uint8_t version) const;

        public:
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

			uint256 DigestSecretaryElectionUnsigned(uint8_t version) const;

			uint256 DigestSecretaryElectionCRCouncilMemberUnsigned(uint8_t version) const;

        public:
			// ReserveCustomID
			void SerializeReserveCustomIDUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeReserveCustomIDUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeReserveCustomIDCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

			bool DeserializeReserveCustomIDCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeReserveCustomID(ByteStream &stream, uint8_t version) const;

            bool DeserializeReserveCustomID(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJsonReserveCustomIDOwnerUnsigned(uint8_t version) const;

            void FromJsonReserveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version);

            nlohmann::json ToJsonReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

            void FromJsonReserveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

            bool IsValidReserveCustomIDOwnerUnsigned(uint8_t version) const;

            bool IsValidReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

            uint256 DigestReserveCustomIDOwnerUnsigned(uint8_t version) const;

            uint256 DigestReserveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

        public:
			// ReceiveCustomID
            void SerializeReceiveCustomIDUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeReceiveCustomIDUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeReceiveCustomIDCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeReceiveCustomIDCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

			void SerializeReceiveCustomID(ByteStream &stream, uint8_t version) const;

			bool DeserializeReceiveCustomID(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonReceiveCustomIDOwnerUnsigned(uint8_t version) const;

			void FromJsonReceiveCustomIDOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonReceiveCustomIDCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidReceiveCustomIDOwnerUnsigned(uint8_t version) const;

			bool IsValidReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

			uint256 DigestReceiveCustomIDOwnerUnsigned(uint8_t version) const;

			uint256 DigestReceiveCustomIDCRCouncilMemberUnsigned(uint8_t version) const;

        public:
			// ChangeCustomIDFee
            void SerializeChangeCustomIDFeeUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeChangeCustomIDFeeUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeChangeCustomIDFeeCRCCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeChangeCustomIDFeeCRCCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeChangeCustomIDFee(ByteStream &stream, uint8_t version) const;

			bool DeserializeChangeCustomIDFee(const ByteStream &stream, uint8_t version);

			nlohmann::json ToJsonChangeCustomIDFeeOwnerUnsigned(uint8_t version) const;

			void FromJsonChangeCustomIDFeeOwnerUnsigned(const nlohmann::json &j, uint8_t version);

			nlohmann::json ToJsonChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const;

			void FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

			bool IsValidChangeCustomIDFeeOwnerUnsigned(uint8_t version) const;

			bool IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const;

            uint256 DigestChangeCustomIDFeeOwnerUnsigned(uint8_t version) const;

            uint256 DigestChangeCustomIDFeeCRCouncilMemberUnsigned(uint8_t version) const;

        public:
            // register side-chain
            void SerializeRegisterSidechainUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeRegisterSidechainUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeRegisterSidechainCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

            bool DeserializeRegisterSidechainCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

            void SerializeRegisterSidechain(ByteStream &stream, uint8_t version) const;

            bool DeserializeRegisterSidechain(const ByteStream &stream, uint8_t version);

            nlohmann::json ToJsonRegisterSidechainUnsigned(uint8_t version) const;

            void FromJsonRegisterSidechainUnsigned(const nlohmann::json &j, uint8_t version);

            nlohmann::json ToJsonRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const;

            void FromJsonRegisterSidechainCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

            bool IsValidRegisterSidechainUnsigned(uint8_t version) const;

            bool IsValidRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const;

            uint256 DigestRegisterSidechainUnsigned(uint8_t version) const;

            uint256 DigestRegisterSidechainCRCouncilMemberUnsigned(uint8_t version) const;

		public:
		    // upgrade code
            void SerializeUpgradeCodeUnsigned(ByteStream &stream, uint8_t version) const;

		    bool DeserializeUpgradeCodeUnsigned(const ByteStream &stream, uint8_t version);

		    void SerializeUpgradeCodeCRCouncilMemberUnsigned(ByteStream &stream, uint8_t version) const;

		    bool DeserializeUpgradeCodeCRCouncilMemberUnsigned(const ByteStream &stream, uint8_t version);

		    void SerializeUpgradeCode(ByteStream &stream, uint8_t version) const;

		    bool DeserializeUpgradeCode(const ByteStream &stream, uint8_t version);

		    nlohmann::json ToJsonUpgradeCodeUnsigned(uint8_t version) const;

		    void FromJsonUpgradeCode(const nlohmann::json &j, uint8_t version);

		    nlohmann::json ToJsonUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const;

		    void FromJsonUpgradeCodeCRCouncilMemberUnsigned(const nlohmann::json &j, uint8_t version);

		    bool IsValidUpgradeCodeUnsigned(uint8_t version) const;

		    bool IsValidUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const;

		    uint256 DigestUpgradeCodeUnsigned(uint8_t version) const;

		    uint256 DigestUpgradeCodeCRCouncilMemberUnsigned(uint8_t version) const;

        public:
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

			bool Equal(const IPayload &payload, uint8_t version) const override;

        private:
		    std::string EncodeDraftData(const bytes_t &draftData) const;

		    bytes_t CheckAndDecodeDraftData(const std::string &draftData, const uint256 &draftHash) const;

		private:
			CRCProposal::Type _type;
			std::string _categoryData;
			bytes_t _ownerPublicKey;
			uint256 _draftHash;
			bytes_t _draftData;
			std::vector <Budget> _budgets;
			Address _recipient;
			uint256 _targetProposalHash;
			std::vector<std::string> _reservedCustomIDList;
			std::vector<std::string> _receivedCustomIDList;
			Address _receiverDID;
            CustomIDFeeRateInfo _customIDFeeRateInfo;
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

			// upgrade code info
			UpgradeCodeInfo _upgradeCodeInfo;

			SideChainInfo _sidechainInfo;
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
