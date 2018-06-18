// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include <boost/shared_ptr.hpp>

#include "Wrapper.h"
#include "CMemBlock.h"
#include "SharedWrapperList.h"
#include "TransactionOutput.h"
#include "Key.h"
#include "WrapperList.h"
#include "Program.h"
#include "ELATransaction.h"
#include "ELAMessageSerializable.h"
#include "ELACoreExt/Attribute.h"
#include "ELACoreExt/Payload/IPayload.h"
#include "ELACoreExt/ELATransaction.h"


namespace Elastos {
	namespace SDK {

		class Transaction :
				public Wrapper<BRTransaction>,
				public ELAMessageSerializable {

		public:
			Transaction();

			Transaction(ELATransaction *transaction, bool manageRaw = true);

			Transaction(const ELATransaction &tx);

			Transaction(const CMBlock &buffer);

			Transaction(const CMBlock &buffer, uint32_t blockHeight, uint32_t timeStamp);

			~Transaction();

			virtual std::string toString() const;

			virtual BRTransaction *getRaw() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			bool isRegistered() const;

			bool &isRegistered();

			UInt256 getHash() const;

			void resetHash();

			uint32_t getVersion() const;

			std::vector<std::string> getInputAddresses();

			const SharedWrapperList<TransactionOutput, BRTxOutput *> &getOutputs() const;

			std::vector<std::string> getOutputAddresses();

			void setTransactionType(ELATransaction::Type type);

			ELATransaction::Type getTransactionType() const;

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

			void addInput(const UInt256 &hash, uint32_t index, uint64_t amount,
							 const CMBlock script, const CMBlock signature, uint32_t sequence);

			void addOutput(TransactionOutput *output);

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


			bool sign(const WrapperList<Key, BRKey> &keys, int forkId);

			bool sign(const Key &key, int forkId);

			/**
			 * Return true if this transaction satisfied the rules in:
			 *      https://bitcoin.org/en/developer-guide#standard-transactions
			 *
			 * @return true if standard; false otherwise
			 */
			bool isStandard();

			UInt256 getReverseHash();

			virtual nlohmann::json toJson();

			virtual void fromJson(nlohmann::json jsonData);

			static uint64_t getMinOutputAmount();

			const PayloadPtr &getPayload() const;

			void addProgram(const ProgramPtr &program);

			const std::vector<AttributePtr> &getAttributes() const;

			const std::vector<ProgramPtr> &getPrograms() const;

		private:
			PayloadPtr newPayload(ELATransaction::Type type);

			void serializeUnsigned(ByteStream &ostream) const;

			nlohmann::json rawTransactionToJson();

			void rawTransactionFromJson(nlohmann::json jsonData);

			bool transactionSign(int forkId, const WrapperList<Key, BRKey> keys);

		private:
			bool _isRegistered;
			bool _manageRaw;
			ELATransaction *_transaction;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
