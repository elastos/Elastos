// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include "Program.h"
#include "TransactionOutput.h"
#include "Attribute.h"
#include "TransactionInput.h"

#include <SDK/Crypto/Key.h>
#include <SDK/Wrapper/WrapperList.h>
#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Plugin/Transaction/Payload/IPayload.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class TransactionHub;

		class Transaction :
				public ELAMessageSerializable {
		public:
			enum Type {
				CoinBase                = 0x00,
				RegisterAsset           = 0x01,
				TransferAsset           = 0x02,
				Record                  = 0x03,
				Deploy                  = 0x04,
				SideChainPow            = 0x05,
				RechargeToSideChain     = 0x06,
				WithdrawFromSideChain   = 0x07,
				TransferCrossChainAsset = 0x08,

				RegisterProducer        = 0x09,
				CancelProducer          = 0x0a,
				UpdateProducer          = 0x0b,
				ReturnDepositCoin       = 0x0c,

				IllegalProposalEvidence = 0x0d,
				IllegalVoteEvidence     = 0x0e,
				IllegalBlockEvidence    = 0x0f,

				RegisterIdentification  = 0xFF, // will refactor later
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

			virtual bool Deserialize(ByteStream &istream);

			uint64_t calculateFee(uint64_t feePerKb);

			uint64_t getTxFee(const boost::shared_ptr<TransactionHub> &wallet);

			bool isRegistered() const;

			bool &isRegistered();

			const UInt256 &getHash() const;

			void resetHash();

			const TxVersion &getVersion() const;

			void setVersion(const TxVersion &version);

			const std::vector<TransactionOutput> &getOutputs() const;

			std::vector<TransactionOutput> &getOutputs();

			void SetOutputs(const std::vector<TransactionOutput> &outputs);

			void AddOutput(const TransactionOutput &output);

			const std::vector<TransactionInput> &getInputs() const;

			std::vector<TransactionInput> &getInputs();

			void AddInput(const TransactionInput &input);

			bool ContainInput(const UInt256 &hash, uint32_t n) const;

			std::vector<std::string> getOutputAddresses();

			void setTransactionType(Type type, const PayloadPtr &payload = nullptr);

			Type getTransactionType() const;

			uint32_t getLockTime() const;

			void setLockTime(uint32_t lockTime);

			uint32_t getBlockHeight() const;

			void setBlockHeight(uint32_t height);

			uint32_t getTimestamp() const;

			void setTimestamp(uint32_t timestamp);

			void addOutput(const TransactionOutput &output);

			void removeChangeOutput();

			void addInput(const TransactionInput &input);

			size_t getSize();

			bool isSigned() const;

			UInt256 getReverseHash();

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			static uint64_t getMinOutputAmount();

			const IPayload *getPayload() const;

			IPayload *getPayload();

			void SetPayload(const PayloadPtr &payload);

			void addAttribute(const Attribute &attribute);

			void addProgram(const Program &program);

			void clearPrograms();

			const std::vector<Attribute> &getAttributes() const;

			const std::vector<Program> &getPrograms() const;

			std::vector<Program> &getPrograms();

			const std::string getRemark() const;

			void setRemark(const std::string &remark);

			nlohmann::json GetSummary(const boost::shared_ptr<TransactionHub> &wallet, uint32_t confirms, bool detail);

			uint8_t	getPayloadVersion() const;

			void setPayloadVersion(uint8_t version);

			uint64_t getFee() const;

			void setFee(uint64_t fee);

			const std::string &GetAssetTableID() const;

			void SetAssetTableID(const std::string &assetTableID);

			void removeDuplicatePrograms();

			void serializeUnsigned(ByteStream &ostream) const;

			UInt256 GetShaData() const;

			UInt256 GetAssetID() const;

			void Cleanup();

			void initPayloadFromType(Type type);

			bool IsEqual(const Transaction *tx) const;

			uint32_t GetConfirms(uint32_t walletBlockHeight) const;

			bool Sign(const std::vector<Key> &keys, const boost::shared_ptr<TransactionHub> &wallet);

		private:

			void reinit();


		private:
			bool _isRegistered;
			mutable UInt256 _txHash;
			std::string _assetTableID;

			TxVersion _version; // uint8_t
			uint32_t _lockTime;
			uint32_t _blockHeight;
			uint32_t _timestamp; // time interval since unix epoch
			Type _type; // uint8_t
			uint8_t _payloadVersion;
			uint64_t _fee;
			PayloadPtr _payload;
			std::vector<TransactionOutput> _outputs;
			std::vector<TransactionInput> _inputs;
			std::vector<Attribute> _attributes;
			std::vector<Program> _programs;
			std::string _remark;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
