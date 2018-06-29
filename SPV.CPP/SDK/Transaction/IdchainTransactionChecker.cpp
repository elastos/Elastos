// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdchainTransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		IdchainTransactionChecker::IdchainTransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet)
				: SidechainTransactionChecker(transaction, wallet) {

		}

		IdchainTransactionChecker::~IdchainTransactionChecker() {

		}

		void IdchainTransactionChecker::Check() {
			if (_transaction->getTransactionType() != ELATransaction::RegisterIdentification &&
				_transaction->getTransactionType() != ELATransaction::TransferAsset &&
				_transaction->getTransactionType() != ELATransaction::TransferCrossChainAsset) {
				throw std::logic_error("MainchainSubWallet transaction type error");
			}
			TransactionChecker::Check();
		}

		bool IdchainTransactionChecker::checkTransactionOutput(const TransactionPtr &transaction) {
			SidechainTransactionChecker::checkTransactionOutput(transaction);

			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}
			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (output->getAddress().empty()) {
					return false;
				}

				if (output->getAmount() != 0) {
					return false;
				}

			}
			return true;
		}
	}
}
