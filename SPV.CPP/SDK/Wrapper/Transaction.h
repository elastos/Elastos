// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include <boost/shared_ptr.hpp>

#include "BRTransaction.h"

#include "Wrapper.h"
#include "ByteData.h"
#include "SharedWrapperList.h"
#include "TransactionInput.h"
#include "TransactionOutput.h"
#include "Key.h"
#include "WrapperList.h"

namespace Elastos {
	namespace SDK {

		class Transaction :
				public Wrapper<BRTransaction *> {
		public:

			Transaction();

			Transaction(BRTransaction *transaction);

			Transaction(const ByteData &buffer);

			Transaction(const ByteData &buffer, uint32_t blockHeight, uint32_t timeStamp);

			~Transaction();

			virtual std::string toString() const;

			virtual BRTransaction *getRaw() const;

			bool isRegistered() const;

			bool &isRegistered();

			ByteData getHash() const;

			uint32_t getVersion() const;

			SharedWrapperList<TransactionInput, BRTxInput *> getInputs();

			std::vector<std::string> getInputAddresses();

			SharedWrapperList<TransactionOutput, BRTxOutput *> getOutputs();

			std::vector<std::string> getOutputAddresses();

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

			static long getMinOutputAmount();

		private:
			void transactionInputCopy(BRTxInput *target, const BRTxInput *source);

			void transactionOutputCopy (BRTxOutput *target, const BRTxOutput *source);

		private:
			bool _isRegistered;

			BRTransaction *_transaction;
		};

		typedef boost::shared_ptr<Transaction> TransactionPtr;

	}
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
