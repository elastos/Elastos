// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdchainTransactionChecker.h"
#include <SDK/Common/ParamChecker.h>

namespace Elastos {
	namespace ElaWallet {

		IdchainTransactionChecker::IdchainTransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet)
			: SidechainTransactionChecker(transaction, wallet) {

		}

		IdchainTransactionChecker::~IdchainTransactionChecker() {

		}

		void IdchainTransactionChecker::Check() {
			ParamChecker::checkCondition(
				_transaction->getTransactionType() != Transaction::RegisterIdentification &&
				_transaction->getTransactionType() != Transaction::TransferAsset &&
				_transaction->getTransactionType() != Transaction::TransferCrossChainAsset,
				Error::Transaction, "Main chain sub wallet tx type error");
			TransactionChecker::Check();
		}

		bool IdchainTransactionChecker::checkTransactionOutput(const TransactionPtr &transaction) {
			SidechainTransactionChecker::checkTransactionOutput(transaction);

			const std::vector<TransactionOutput> &outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}

			std::vector<std::string> addresses = _wallet->getAllAddresses();

			for (size_t i = 0; i < size; ++i) {
				const TransactionOutput &output = outputs[i];
				if (output.getAddress().empty()) {
					return false;
				}
				if (std::find(addresses.begin(), addresses.end(), output.getAddress()) == addresses.end()) {
					if (output.getAmount() != 0) {
						return false;
					}
				}
			}
			return true;
		}
	}
}
