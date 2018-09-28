// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ParamChecker.h>
#include "SidechainTransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		SidechainTransactionChecker::SidechainTransactionChecker(const TransactionPtr &transaction,
																 const WalletPtr &wallet)
			: TransactionChecker(transaction, wallet) {

		}

		SidechainTransactionChecker::~SidechainTransactionChecker() {

		}

		void SidechainTransactionChecker::Check() {

			ParamChecker::checkCondition(
				_transaction->getTransactionType() != Transaction::TransferCrossChainAsset &&
				_transaction->getTransactionType() != Transaction::TransferAsset,
				Error::Transaction, "Side chain sub wallet tx type error");
			TransactionChecker::Check();
		}
	}
}