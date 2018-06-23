// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Payload/PayloadTransferCrossChainAsset.h"
#include "MainchainTransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		MainchainTransactionChecker::MainchainTransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet)
				: TransactionChecker(transaction, wallet) {

		}

		MainchainTransactionChecker::~MainchainTransactionChecker() {

		}

		void MainchainTransactionChecker::Check() {
			if (_transaction->getTransactionType() != ELATransaction::TransferCrossChainAsset &&
				_transaction->getTransactionType() != ELATransaction::TransferAsset) {
				throw std::logic_error("MainchainSubWallet transaction type error");
			}

			TransactionChecker::Check();
		}

		bool MainchainTransactionChecker::checkTransactionPayload(const TransactionPtr &transaction) {
			const PayloadPtr payloadPtr = transaction->getPayload();

			bool isValid = TransactionChecker::checkTransactionPayload(transaction);

			if (isValid) {
				PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
						static_cast<PayloadTransferCrossChainAsset *>(payloadPtr.get());

				std::vector<uint64_t> outputIndex = payloadTransferCrossChainAsset->getOutputIndex();
				const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
				for (size_t i = 0; i < outputIndex.size(); ++i) {
					if (outputIndex[i] > outputs.size() - 1) {
						isValid = false;
						break;
					}
				}

			}
			return isValid;
		}
	}
}