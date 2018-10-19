// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
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
			TransactionPtr resultTx = _transaction;

			std::string outAddr = _transaction->getOutputs()[0]->getAddress();
			uint64_t inputAmount = getInputsAmount(_transaction);
			uint64_t outputAmount = _transaction->getOutputs()[0]->getAmount();
			uint64_t chargeAmount = _transaction->getOutputs().size() > 1
									? _transaction->getOutputs()[1]->getAmount() : 0;

			if (inputAmount - outputAmount - chargeAmount == actualFee) {
				return resultTx;
			}

			if (inputAmount > outputAmount && inputAmount - outputAmount >= actualFee) {
				modifyTransactionChange(resultTx, inputAmount - outputAmount - actualFee);
			} else {
				resultTx = recreateTransaction(actualFee, outputAmount, outAddr, resultTx->getRemark(),
											   getMemo());
			}

			const std::vector<TransactionOutput *> &resultOutputs = resultTx->getOutputs();
			inputAmount = getInputsAmount(resultTx);
			outputAmount = resultOutputs[0]->getAmount();
			chargeAmount = resultOutputs.size() > 1 ? resultOutputs[1]->getAmount() : 0;
			resultTx->setFee(inputAmount - outputAmount - chargeAmount);
			completedTransactionAssetID(resultTx);
			completedTransactionPayload(resultTx);
			resultTx->resetHash();
			return resultTx;
		}

		void TransactionCompleter::completedTransactionAssetID(const TransactionPtr &transaction) {
			const std::vector<TransactionOutput *> &outputs = transaction->getOutputs();
			size_t size = outputs.size();
			ParamChecker::checkCondition(size < 1, Error::Transaction,
										 "Tx complete asset ID without output");

			UInt256 zero = UINT256_ZERO;
			UInt256 assetID = Key::getSystemAssetId();

			for (size_t i = 0; i < size; ++i) {
				TransactionOutput *output = outputs[i];
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

		TransactionPtr
		TransactionCompleter::recreateTransaction(uint64_t fee, uint64_t amount, const std::string &toAddress,
												  const std::string &remark, const std::string &memo) {
			return _wallet->createTransaction("", fee, amount, toAddress, remark, memo);
		}

		void TransactionCompleter::modifyTransactionChange(const TransactionPtr &transaction, uint64_t actualChange) {
			const std::vector<TransactionOutput *> &outputs = transaction->getOutputs();
			if (actualChange > 0) {
				if (outputs.size() >= 2) {
					outputs[1]->setAmount(actualChange);
				} else if (outputs.size() == 1) {
					std::string changeAddress = _wallet->getAllAddresses()[0];
					TransactionOutput *output = new TransactionOutput;
					output->setAddress(changeAddress);
					output->setAmount(actualChange);
					output->setAssetId(Key::getSystemAssetId());
					output->setOutputLock(0);
					UInt168 u168Address = UINT168_ZERO;
					Utils::UInt168FromAddress(u168Address, changeAddress);
					output->setProgramHash(u168Address);
					transaction->addOutput(output);
				}
			} else {
				transaction->removeChargeOutput();
			}
		}

		std::string TransactionCompleter::getMemo() const {
			for (int i = 0; i < _transaction->getAttributes().size(); ++i) {
				if (_transaction->getAttributes()[i]->GetUsage() == Attribute::Memo)
					return Utils::convertToString(_transaction->getAttributes()[i]->GetData());
			}
			return "";
		}

	}
}