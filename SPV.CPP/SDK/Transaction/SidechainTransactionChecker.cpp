// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainTransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		SidechainTransactionChecker::SidechainTransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet)
				: TransactionChecker(transaction, wallet) {

		}

		SidechainTransactionChecker::~SidechainTransactionChecker() {

		}

		void SidechainTransactionChecker::Check() {

			if (_transaction->getTransactionType() != ELATransaction::TransferCrossChainAsset &&
				_transaction->getTransactionType() != ELATransaction::TransferAsset) {
				throw std::logic_error("SidechainSubWallet transaction type error");
			}
			TransactionChecker::Check();
		}
	}
}