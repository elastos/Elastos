// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONCOMPLETER_H__
#define __ELASTOS_SDK_TRANSACTIONCOMPLETER_H__

#include "Wallet.h"
#include "Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class TransactionCompleter {
		public:
			TransactionCompleter(const TransactionPtr &transaction, const WalletPtr &wallet);

			virtual ~TransactionCompleter();

			virtual TransactionPtr Complete(uint64_t actualFee);

		protected:
			virtual TransactionPtr
			recreateTransaction(uint64_t fee, uint64_t amount, const std::string &toAddress, const std::string &remark,
								const std::string &memo, const UInt256 &assetID);

			virtual void modifyTransactionChange(const TransactionPtr &transaction, uint64_t actualChange);

			virtual void completedTransactionAssetID(const TransactionPtr &transaction);

			virtual void completedTransactionPayload(const TransactionPtr &transaction);

			uint64_t getInputsAmount(const TransactionPtr &transaction) const;

			std::string getMemo() const;

		protected:
			WalletPtr _wallet;
			TransactionPtr _transaction;
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONCOMPLETER_H__
