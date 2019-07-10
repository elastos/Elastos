// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include "Program.h"
#include "TransactionOutput.h"
#include "Attribute.h"
#include "TransactionInput.h"

#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Plugin/Transaction/Payload/IPayload.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Wallet;

		class Transaction :
				public ELAMessageSerializable {
		public:
			enum Type {
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

				registerIdentification   = 0xFF, // will refactor later
				TypeMaxCount
			};

			enum TxVersion {
				Default = 0x00,
				V09 = 0x09,
			};

		public:
			Transaction();

			Transaction(const Transaction &tx);

			Transaction &operator=(const Transaction &tx);

			~Transaction();

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			uint64_t CalculateFee(uint64_t feePerKb);

			uint64_t GetTxFee(const boost::shared_ptr<Wallet> &wallet);

			bool IsRegistered() const;

			bool &IsRegistered();

			const uint256 &GetHash() const;

			void ResetHash();

			const TxVersion &GetVersion() const;

			void SetVersion(const TxVersion &version);

			const std::vector<TransactionOutput> &GetOutputs() const;

			std::vector<TransactionOutput> &GetOutputs();

			void SetOutputs(const std::vector<TransactionOutput> &outputs);

			void AddOutput(const TransactionOutput &output);

			const std::vector<TransactionInput> &GetInputs() const;

			std::vector<TransactionInput> &GetInputs();

			void AddInput(const TransactionInput &Input);

			bool ContainInput(const uint256 &hash, uint32_t n) const;

			void SetTransactionType(Type type, const PayloadPtr &payload = nullptr);

			Type GetTransactionType() const;

			uint32_t GetLockTime() const;

			void SetLockTime(uint32_t lockTime);

			uint32_t GetBlockHeight() const;

			void SetBlockHeight(uint32_t height);

			time_t GetTimestamp() const;

			void SetTimestamp(time_t timestamp);

			size_t EstimateSize() const;

//			size_t GetSize();

			nlohmann::json GetSignedInfo() const;

			bool IsSigned() const;

			bool IsCoinBase() const;

			bool IsValid() const;

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

			static uint64_t GetMinOutputAmount();

			const IPayload *GetPayload() const;

			IPayload *GetPayload();

			void SetPayload(const PayloadPtr &payload);

			void AddAttribute(const Attribute &attribute);

			bool AddUniqueProgram(const Program &program);

			void AddProgram(const Program &program);

			void ClearPrograms();

			const std::vector<Attribute> &GetAttributes() const;

			const std::vector<Program> &GetPrograms() const;

			std::vector<Program> &GetPrograms();

			nlohmann::json GetSummary(const boost::shared_ptr<Wallet> &wallet, uint32_t confirms, bool detail);

			uint8_t	GetPayloadVersion() const;

			void SetPayloadVersion(uint8_t version);

			uint64_t GetFee() const;

			void SetFee(uint64_t fee);

			void SerializeUnsigned(ByteStream &ostream) const;

			uint256 GetShaData() const;

			void Cleanup();

			void InitPayloadFromType(Type type);

			bool IsEqual(const Transaction *tx) const;

			uint32_t GetConfirms(uint32_t walletBlockHeight) const;

		private:

			void Reinit();


		private:
			bool _isRegistered;
			mutable uint256 _txHash;
			std::string _assetTableID;

			TxVersion _version; // uint8_t
			uint32_t _lockTime;
			uint32_t _blockHeight;
			time_t _timestamp; // time interval since unix epoch
			Type _type; // uint8_t
			uint8_t _payloadVersion;
			uint64_t _fee;
			PayloadPtr _payload;
			std::vector<TransactionOutput> _outputs;
			std::vector<TransactionInput> _inputs;
			std::vector<Attribute> _attributes;
			std::vector<Program> _programs;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
