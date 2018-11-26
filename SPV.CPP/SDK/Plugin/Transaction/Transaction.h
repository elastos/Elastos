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
				CoinBase = 0x00,
				RegisterAsset = 0x01,
				TransferAsset = 0x02,
				Record = 0x03,
				Deploy = 0x04,
				SideMining = 0x05,
				IssueToken = 0x06,
				WithdrawAsset = 0x07,
				TransferCrossChainAsset = 0x08,
				RegisterIdentification = 0x09,
				TypeMaxCount
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

			uint32_t getVersion() const;

			void setVersion(uint32_t version);

			const std::vector<TransactionOutput> &getOutputs() const;

			std::vector<TransactionOutput> &getOutputs();

			const std::vector<TransactionInput> &getInputs() const;

			std::vector<TransactionInput> &getInputs();

			std::vector<std::string> getOutputAddresses();

			void setTransactionType(Type type);

			Type getTransactionType() const;

			uint32_t getLockTime();

			void setLockTime(uint32_t lockTime);

			uint32_t getBlockHeight();

			void setBlockHeight(uint32_t height);

			uint32_t getTimestamp();

			void setTimestamp(uint32_t timestamp);

			void addOutput(const TransactionOutput &output);

			void removeChangeOutput();

			void addInput(const TransactionInput &input);

			size_t getSize();

			bool isSigned() const;

			bool sign(const WrapperList<Key, BRKey> &keys, const boost::shared_ptr<TransactionHub> &wallet);

			UInt256 getReverseHash();

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			static uint64_t getMinOutputAmount();

			const IPayload *getPayload() const;

			IPayload *getPayload();

			void addAttribute(const Attribute &attribute);

			void addProgram(const Program &program);

			void clearPrograms();

			const std::vector<Attribute> &getAttributes() const;

			const std::vector<Program> &getPrograms() const;

			std::vector<Program> &getPrograms();

			const std::string getRemark() const;

			void setRemark(const std::string &remark);

			nlohmann::json generateExtraTransactionInfo(const boost::shared_ptr<TransactionHub> &wallet,
											  uint32_t lastBlockHeight);

			uint8_t	getPayloadVersion() const;

			void setPayloadVersion(uint8_t version);

			uint64_t getFee() const;

			void setFee(uint64_t fee);

			const std::string &GetAssetTableID() const;

			void SetAssetTableID(const std::string &assetTableID);

			void removeDuplicatePrograms();

			void serializeUnsigned(ByteStream &ostream) const;

			CMBlock GetShaData() const;

			UInt256 GetAssetID() const;

			void Cleanup();

			void initPayloadFromType(Type type);

			bool IsEqual(const Transaction *tx) const;

		private:

			void reinit();

			bool transactionSign(const WrapperList<Key, BRKey> keys, const boost::shared_ptr<TransactionHub> &wallet);

			std::string getConfirmInfo(uint32_t lastBlockHeight);

			std::string getStatus(uint32_t lastBlockHeight);

		private:
			bool _isRegistered;
			mutable UInt256 _txHash;
			std::string _assetTableID;

			uint32_t _version;
			uint32_t _lockTime;
			uint32_t _blockHeight;
			uint32_t _timestamp; // time interval since unix epoch
			Type _type;
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
