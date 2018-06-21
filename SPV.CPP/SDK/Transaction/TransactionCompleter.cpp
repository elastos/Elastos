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

		void TransactionCompleter::Complete(uint64_t actualFee) {
			if (_transaction->getRaw()->inCount <= 0) {
				completedTransactionInputs(_transaction);
			}

			uint64_t inputFee = _wallet->getInputsFee(_transaction);
			uint64_t outputFee = _wallet->getOutputFee(_transaction);

			if (inputFee - outputFee > 0) {
				completedTransactionOutputs(_transaction, inputFee - outputFee);
			}

			completedTransactionAssetID(_transaction);
			completedTransactionPayload(_transaction);
		}

		void TransactionCompleter::completedTransactionInputs(const TransactionPtr &transaction) {
			BRWallet *wallet = _wallet->getRaw();
			BRUTXO *o = nullptr;
			ELATransaction *tx = nullptr;
			for (size_t i = 0; i < array_count(wallet->utxos); i++) {
				o = &wallet->utxos[i];
				tx = (ELATransaction *) BRSetGet(wallet->allTx, o);
				if (!tx || o->n >= tx->raw.outCount) continue;

				CMBlock script = tx->outputs[o->n]->getScript();
				transaction->addInput(tx->raw.txHash, o->n, tx->outputs[o->n]->getAmount(), script, CMBlock(),
									  TXIN_SEQUENCE);
			}
		}

		void TransactionCompleter::completedTransactionOutputs(const TransactionPtr &transaction, uint64_t amount) {
			ELATxOutput *o = new ELATxOutput();
			uint64_t maxAmount = _wallet->getMaxOutputAmount();
			o->raw.amount = (amount < maxAmount) ? amount : maxAmount;
			std::string addr = _wallet->getReceiveAddress();
			memcpy(o->raw.address, addr.data(), addr.size());

			BRTxOutputSetAddress((BRTxOutput *) o, addr.c_str());

			TransactionOutput *output = new TransactionOutput(o);
			transaction->addOutput(output);
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
		
	}
}