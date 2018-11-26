// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionCompleter.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Plugin/Transaction/Asset.h>

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

			std::string outAddr = _transaction->getOutputs()[0].getAddress();
			uint64_t inputAmount = getInputsAmount(_transaction);
			uint64_t outputAmount = _transaction->getOutputs()[0].getAmount();
			uint64_t changeAmount = _transaction->getOutputs().size() > 1
									? _transaction->getOutputs()[1].getAmount() : 0;

			if (inputAmount - outputAmount - changeAmount == actualFee) {
				return resultTx;
			}

			if (inputAmount > outputAmount && inputAmount - outputAmount >= actualFee) {
				modifyTransactionChange(resultTx, inputAmount - outputAmount - actualFee);
			} else {
				resultTx = recreateTransaction(actualFee, outputAmount, outAddr, resultTx->getRemark(),
											   getMemo(), _transaction->GetAssetID());
			}

			const std::vector<TransactionOutput> &resultOutputs = resultTx->getOutputs();
			inputAmount = getInputsAmount(resultTx);
			outputAmount = resultOutputs[0].getAmount();
			changeAmount = resultOutputs.size() > 1 ? resultOutputs[1].getAmount() : 0;
			resultTx->setFee(inputAmount - outputAmount - changeAmount);
			completedTransactionAssetID(resultTx);
			completedTransactionPayload(resultTx);
			resultTx->resetHash();
			return resultTx;
		}

		void TransactionCompleter::completedTransactionAssetID(const TransactionPtr &transaction) {
			ParamChecker::checkCondition(transaction->getOutputs().size() < 1, Error::Transaction,
										 "Tx complete asset ID without output");

			UInt256 zero = UINT256_ZERO;
			UInt256 assetID = Asset::GetELAAssetID();

			for (size_t i = 0; i < transaction->getSize(); ++i) {
				if (UInt256Eq(&transaction->getOutputs()[0].getAssetId(), &zero) == 1) {
					transaction->getOutputs()[0].setAssetId(assetID);
				}
			}
		}

		void TransactionCompleter::completedTransactionPayload(const TransactionPtr &transaction) {

		}

		uint64_t TransactionCompleter::getInputsAmount(const TransactionPtr &transaction) const {
			uint64_t amount = 0;

			for (size_t i = 0; i < transaction->getInputs().size(); i++) {
				UInt256 hash = transaction->getInputs()[i].getTransctionHash();
				const TransactionPtr &t = _wallet->transactionForHash(hash);
				uint32_t n = transaction->getInputs()[i].getIndex();

				if (t && n < t->getOutputs().size()) {
					amount += t->getOutputs()[n].getAmount();
				} else
					return UINT64_MAX;
			}

			return amount;
		}

		TransactionPtr
		TransactionCompleter::recreateTransaction(uint64_t fee, uint64_t amount, const std::string &toAddress,
												  const std::string &remark, const std::string &memo,
												  const UInt256 &assetID) {
			return _wallet->createTransaction("", fee, amount, toAddress, assetID, remark, memo);
		}

		void TransactionCompleter::modifyTransactionChange(const TransactionPtr &transaction, uint64_t actualChange) {
			std::vector<TransactionOutput> &outputs = transaction->getOutputs();
			if (actualChange > 0) {
				if (outputs.size() >= 2) {
					outputs[1].setAmount(actualChange);
				} else if (outputs.size() == 1) {
					std::string changeAddress = _wallet->getAllAddresses()[0];
					transaction->addOutput(TransactionOutput(actualChange, changeAddress));
				}
			} else {
				transaction->removeChangeOutput();
			}
		}

		std::string TransactionCompleter::getMemo() const {
			for (int i = 0; i < _transaction->getAttributes().size(); ++i) {
				if (_transaction->getAttributes()[i].GetUsage() == Attribute::Memo)
					return Utils::convertToString(_transaction->getAttributes()[i].GetData());
			}
			return "";
		}

	}
}