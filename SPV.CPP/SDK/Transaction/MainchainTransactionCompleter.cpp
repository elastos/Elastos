// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		MainchainTransactionCompleter::MainchainTransactionCompleter(const TransactionPtr &transaction,
																	 const WalletPtr &wallet) : TransactionCompleter(
				transaction, wallet) {

		}

		MainchainTransactionCompleter::~MainchainTransactionCompleter() {

		}
	}
}