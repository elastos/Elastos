// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainTransactionChecker.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>

namespace Elastos {
	namespace ElaWallet {

		MainchainTransactionChecker::MainchainTransactionChecker(
			const TransactionPtr &transaction,
			const WalletPtr &wallet) : TransactionChecker(transaction, wallet) {

		}

		MainchainTransactionChecker::~MainchainTransactionChecker() {

		}

		void MainchainTransactionChecker::Check() {
			ParamChecker::checkCondition(
				_transaction->getTransactionType() != Transaction::TransferCrossChainAsset &&
				_transaction->getTransactionType() != Transaction::TransferAsset,
				Error::Transaction, "Main chain sub wallet tx type error");

			TransactionChecker::Check();
		}

		bool MainchainTransactionChecker::checkTransactionPayload(const TransactionPtr &transaction) {
			bool isValid = TransactionChecker::checkTransactionPayload(transaction);

			if (isValid && transaction->getTransactionType() == Transaction::Type::TransferCrossChainAsset) {
				const PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
					static_cast<const PayloadTransferCrossChainAsset *>(transaction->getPayload());

				std::vector<uint64_t> outputIndex = payloadTransferCrossChainAsset->getOutputIndex();
				const std::vector<TransactionOutput> &outputs = transaction->getOutputs();
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