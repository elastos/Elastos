// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionCompleter::TransactionCompleter(const TransactionPtr &transaction, const WalletPtr &wallet) :
				_transaction(transaction),
				_wallet(wallet) {
		}

		TransactionCompleter::~TransactionCompleter() {

		}

		TransactionPtr TransactionCompleter::Complete(uint64_t actualFee) {
			std::string outAddr;
			uint64_t inputAmount = getInputsAmount(_transaction);
			uint64_t outputAmount = getOutputAmount(_transaction, outAddr);

			TransactionPtr resultTx = _transaction;
			if (inputAmount > outputAmount && inputAmount - outputAmount >= actualFee) {
				modifyTransactionChange(resultTx, inputAmount - outputAmount - actualFee);
			} else {
				resultTx = recreateTransaction(actualFee, outputAmount, outAddr);
			}

			completedTransactionAssetID(resultTx);
			completedTransactionPayload(resultTx);
			return resultTx;
		}

		void TransactionCompleter::completedTransactionAssetID(const TransactionPtr &transaction) {
			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				throw std::logic_error("completedTransactionAssetID transaction has't outputs");
			}

			UInt256 zero = UINT256_ZERO;
			UInt256 assetID = Key::getSystemAssetId();

			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (UInt256Eq(&output->getAssetId(), &zero) == 1) {
					output->setAssetId(assetID);
				}
			}
		}

		void TransactionCompleter::completedTransactionPayload(const TransactionPtr &transaction) {

		}

		uint64_t TransactionCompleter::getInputsAmount(const TransactionPtr &transaction) const {
			uint64_t amount = 0;

			for (size_t i = 0; i < transaction->getRaw()->inCount; i++) {
				UInt256 hash = transaction->getRaw()->inputs[i].txHash;
				ELATransaction *t = (ELATransaction *) BRWalletTransactionForHash(_wallet->getRaw(), hash);
				uint32_t n = transaction->getRaw()->inputs[i].index;

				if (t && n < t->outputs.size()) {
					amount += t->outputs[n]->getAmount();
				} else
					return UINT64_MAX;
			}

			return amount;
		}

		uint64_t TransactionCompleter::getOutputAmount(const TransactionPtr &transaction, std::string &address) const {
			uint64_t amount = 0;

			std::vector<std::string> addresses = _wallet->getAllAddresses();
			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			for (size_t i = 0; i < outputs.size(); i++) {
				//filter the output for change
				if (std::find(addresses.begin(), addresses.end(), outputs[i]->getAddress()) == addresses.end()) {
					amount += outputs[i]->getAmount();
					address = outputs[i]->getAddress();
				}
			}

			return amount;
		}

		TransactionPtr
		TransactionCompleter::recreateTransaction(uint64_t fee, uint64_t amount, const std::string &toAddress) {
			return _wallet->createTransaction("", fee, amount, toAddress);
		}

		void TransactionCompleter::modifyTransactionChange(const TransactionPtr &transaction, uint64_t actualChange) {
			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			std::vector<std::string> addresses = _wallet->getAllAddresses();
			for (size_t i = 0; i < outputs.size(); ++i) {
				if (std::find(addresses.begin(), addresses.end(), outputs[i]->getAddress()) != addresses.end()) {
					outputs[i]->setAmount(actualChange);
				}
			}
		}

	}
}