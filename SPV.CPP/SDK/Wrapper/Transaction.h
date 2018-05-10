// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include <boost/shared_ptr.hpp>

#include "BRTransaction.h"

#include "../ELACoreExt/Payload/IPayload.h"
#include "Wrapper.h"
#include "ByteData.h"
#include "SharedWrapperList.h"
#include "TransactionInput.h"
#include "TransactionOutput.h"
#include "Key.h"
#include "WrapperList.h"
#include "ELAMessageSerializable.h"
#include "../ELACoreExt/Attribute.h"
#include "Program.h"

namespace Elastos {
	namespace SDK {

		class Transaction :
				public Wrapper<BRTransaction>,
				public ELAMessageSerializable {
		public:
			enum Type {
				CoinBase                = 0x00,
				RegisterAsset           = 0x01,
				TransferAsset           = 0x02,
				Record                  = 0x03,
				Deploy                  = 0x04,
				SideMining              = 0x05,
				IssueToken              = 0x06,
				WithdrawAsset           = 0x07,
				TransferCrossChainAsset = 0x08,
			};

		public:
			Transaction();

			Transaction(BRTransaction *transaction);

			Transaction(const ByteData &buffer);

			Transaction(const ByteData &buffer, uint32_t blockHeight, uint32_t timeStamp);

			~Transaction();

			virtual std::string toString() const;

			virtual BRTransaction *getRaw() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			BRTransaction *convertToRaw() const;

			bool isRegistered() const;

			bool &isRegistered();

			UInt256 getHash() const;

			void resetHash();

			uint32_t getVersion() const;

			const SharedWrapperList<TransactionInput, BRTxInput *> &getInputs() const;

			std::vector<std::string> getInputAddresses();

			const SharedWrapperList<TransactionOutput, BRTxOutput *> &getOutputs() const;

			std::vector<std::string> getOutputAddresses();

			void setTransactionType(Transaction::Type type);

			Transaction::Type getTransactionType() const;

			/**
			 * The transaction's lockTime
			 *
			 * @return the lock time as a long (from a uint32_t)
			 */
			uint32_t getLockTime();

			void setLockTime(uint32_t lockTime);

			/**
			 * The transaction's blockHeight.
			 *
			 * @return the blockHeight as a long (from a uint32_t).
			 */
			uint32_t getBlockHeight();

			/**
			 * The transacdtion's timestamp.
			 *
			 * @return the timestamp as a long (from a uint32_t).
			 */
			uint32_t getTimestamp();

			void setTimestamp(uint32_t timestamp);

			ByteData serialize();

			void addInput(const TransactionInput &input);

			void addOutput(const TransactionOutput &output);

			/**
			 * Shuffle the transaction's outputs.
			 */
			void shuffleOutputs();

			/**
			 * The the transactions' size in bytes if signed, or the estimated size assuming
			 * compact pubkey sigs
		
			 * @return the size in bytes.
			 */
			size_t getSize();

			/**
			 * The transaction's standard fee which is the minimum transaction fee needed for the
			 * transaction to relay across the bitcoin network.
			 * *
			 * @return the fee (in Satoshis)?
			 */
			uint64_t getStandardFee();

			/**
			 * Returns true if all the transaction's signatures exists.  This method does not verify
			 * the signatures.
			 *
			 * @return true if all exist.
			 */
			bool isSigned();


			void sign(const WrapperList<Key, BRKey> &keys, int forkId);

			void sign(const Key &key, int forkId);

			/**
			 * Return true if this transaction satisfied the rules in:
			 *      https://bitcoin.org/en/developer-guide#standard-transactions
			 *
			 * @return true if standard; false otherwise
			 */
			bool isStandard();

			UInt256 getReverseHash();

			static uint64_t getMinOutputAmount();

		private:
			void convertFrom(const BRTransaction *raw);

			void transactionInputCopy(BRTxInput *target, const BRTxInput *source) const;

			void transactionOutputCopy (BRTxOutput *target, const BRTxOutput *source) const;

			void setPayloadByTransactionType();

			void serializeUnsigned(ByteStream &ostream) const;
		private:
			bool _isRegistered;

			BRTransaction *_transaction;
			Type _type;
			uint8_t _payloadVersion;
			PayloadPtr _payload;
			std::vector<AttributePtr> _attributes;
			std::vector<ProgramPtr> _programs;
			mutable SharedWrapperList<TransactionInput, BRTxInput *> _inputs;
			mutable SharedWrapperList<TransactionOutput, BRTxOutput *> _outputs;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
