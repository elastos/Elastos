// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include <Common/JsonSerializer.h>
#include <Plugin/Interface/ELAMessageSerializable.h>
#include <Plugin/Transaction/Payload/IPayload.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Wallet;
		class TransactionOutput;
		class TransactionInput;
		class Program;
		class Attribute;
		typedef boost::shared_ptr<Wallet> WalletPtr;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef std::vector<OutputPtr> OutputArray;
		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef std::vector<InputPtr> InputArray;
		typedef boost::shared_ptr<Program> ProgramPtr;
		typedef std::vector<ProgramPtr> ProgramArray;
		typedef boost::shared_ptr<Attribute> AttributePtr;
		typedef std::vector<AttributePtr> AttributeArray;

		class Transaction : public JsonSerializer {
		public:
			enum {
				coinBase                 = 0x00,
				registerAsset            = 0x01,
				transferAsset            = 0x02,
				record                   = 0x03,
				deploy                   = 0x04,
				sideChainPow             = 0x05,
				rechargeToSideChain      = 0x06,
				withdrawFromSideChain    = 0x07,
				transferCrossChainAsset  = 0x08,

				registerProducer         = 0x09,
				cancelProducer           = 0x0a,
				updateProducer           = 0x0b,
				returnDepositCoin        = 0x0c,
				activateProducer         = 0x0d,

				IllegalProposalEvidence  = 0x0e,
				IllegalVoteEvidence      = 0x0f,
				IllegalBlockEvidence     = 0x10,
				IllegalSidechainEvidence = 0x11,
				InactiveArbitrators      = 0x12,
				UpdateVersion            = 0x13,

				registerCR               = 0x21,
				unregisterCR             = 0x22,
				updateCR                 = 0x23,
				returnCRDepositCoin      = 0x24,

				crcProposal              = 0x25,
				crcProposalReview        = 0x26,
				crcProposalTracking      = 0x27,
				crcAppropriation         = 0x28,
				crcProposalWithdraw      = 0x29,

				TypeMaxCount
			};

			enum TxVersion {
				Default = 0x00,
				V09 = 0x09,
			};

		public:
			Transaction();

			Transaction(uint8_t type, const PayloadPtr &payload);

			Transaction(const Transaction &tx);

			Transaction &operator=(const Transaction &tx);

			virtual ~Transaction();

			void Serialize(ByteStream &ostream, bool extend = false) const;

			bool Deserialize(const ByteStream &istream, bool extend = false);

			virtual bool DeserializeType(const ByteStream &istream);

			uint64_t CalculateFee(uint64_t feePerKb);

			uint64_t GetTxFee(const boost::shared_ptr<Wallet> &wallet);

			bool IsRegistered() const;

			bool &IsRegistered();

			const uint256 &GetHash() const;

			void SetHash(const uint256 &hash);

			void ResetHash();

			const TxVersion &GetVersion() const;

			void SetVersion(const TxVersion &version);

			const std::vector<OutputPtr> &GetOutputs() const;

			void FixIndex();

			OutputPtr OutputOfIndex(uint16_t fixedIndex) const;

			void SetOutputs(const std::vector<OutputPtr> &outputs);

			void AddOutput(const OutputPtr &output);

			void RemoveOutput(const OutputPtr &output);

			const std::vector<InputPtr> &GetInputs() const;

			std::vector<InputPtr> &GetInputs();

			void AddInput(const InputPtr &Input);

			bool ContainInput(const uint256 &hash, uint32_t n) const;

			uint8_t GetTransactionType() const;

			virtual bool IsDPoSTransaction() const;

			virtual bool IsCRCTransaction() const;

			virtual bool IsProposalTransaction() const;

			virtual bool IsIDTransaction() const;

			uint32_t GetLockTime() const;

			void SetLockTime(uint32_t lockTime);

			uint32_t GetBlockHeight() const;

			void SetBlockHeight(uint32_t height);

			time_t GetTimestamp() const;

			void SetTimestamp(time_t timestamp);

			size_t EstimateSize() const;

			nlohmann::json GetSignedInfo() const;

			bool IsSigned() const;

			bool IsCoinBase() const;

			bool IsUnconfirmed() const;

			bool IsValid() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			static uint64_t GetMinOutputAmount();

			const IPayload *GetPayload() const;

			IPayload *GetPayload();

			const PayloadPtr &GetPayloadPtr() const;

			void SetPayload(const PayloadPtr &payload);

			void AddAttribute(const AttributePtr &attribute);

			bool AddUniqueProgram(const ProgramPtr &program);

			void AddProgram(const ProgramPtr &program);

			void ClearPrograms();

			const std::vector<AttributePtr> &GetAttributes() const;

			const std::vector<ProgramPtr> &GetPrograms() const;

			nlohmann::json GetSummary(const WalletPtr &wallet, uint32_t confirms, bool detail);

			uint8_t	GetPayloadVersion() const;

			void SetPayloadVersion(uint8_t version);

			uint64_t GetFee() const;

			void SetFee(uint64_t fee);

			void SerializeUnsigned(ByteStream &ostream, bool extend = false) const;

			uint256 GetShaData() const;

			void Cleanup();

			bool IsEqual(const Transaction &tx) const;

			uint32_t GetConfirms(uint32_t walletBlockHeight) const;

			std::string GetConfirmStatus(uint32_t walletBlockHeight) const;

		public:
			virtual PayloadPtr InitPayload(uint8_t type);

		private:

			void Reinit();


		protected:
			bool _isRegistered;
			mutable uint256 _txHash;

			TxVersion _version; // uint8_t
			uint32_t _lockTime;
			uint32_t _blockHeight;
			time_t _timestamp; // time interval since unix epoch
			uint8_t _type;
			uint8_t _payloadVersion;
			uint64_t _fee;
			PayloadPtr _payload;
			OutputArray _outputs;
			InputArray _inputs;
			AttributeArray _attributes;
			ProgramArray _programs;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

namespace nlohmann {
	template<>
	struct adl_serializer<Elastos::ElaWallet::Transaction> {
		static void to_json(json &j, const Elastos::ElaWallet::Transaction &tx) {
			j = tx.ToJson();
		}

		static void from_json(const json &j, Elastos::ElaWallet::Transaction &tx) {
			tx.FromJson(j);
		}
	};
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
