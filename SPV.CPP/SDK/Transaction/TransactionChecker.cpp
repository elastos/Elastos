// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"
#include "TransactionChecker.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionChecker::TransactionChecker(const TransactionPtr &transaction, const WalletPtr &wallet) :
			_transaction(transaction),
			_wallet(wallet) {
		}

		TransactionChecker::~TransactionChecker() {

		}

		void TransactionChecker::Check() {
			if (!checkTransactionOutput(_transaction)) {
				throw std::logic_error("checkTransactionOutput error.");
			}

			if (!checkTransactionAttribute(_transaction)) {
				throw std::logic_error("checkTransactionAttribute error.");
			}

			if (!checkTransactionProgram(_transaction)) {
				throw std::logic_error("checkTransactionProgram error.");
			}

			if (!checkTransactionPayload(_transaction)) {
				throw std::logic_error("checkTransactionPayload error.");
			}
		}

		bool TransactionChecker::checkTransactionOutput(const TransactionPtr &transaction) {

			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}

			bool hasChange = false;
			bool hasOutput = false;
			std::vector<std::string> addresses = _wallet->getAllAddresses();
			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (!Address::UInt168IsValid(output->getProgramHash())) {
					return false;
				}
				if (std::find(addresses.begin(), addresses.end(), output->getAddress()) != addresses.end()) {
					if (hasChange) //should have only one change output per tx
						return false;
					hasChange = true;
				} else {
					if (hasOutput) //todo we support only one output, modify this if we support multi-output later
						return false;
					hasOutput = true;
				}
			}
			return hasOutput;
		}

		bool TransactionChecker::checkTransactionAttribute(const TransactionPtr &transaction) {
			const std::vector<AttributePtr> attributes = transaction->getAttributes();
			size_t size = attributes.size();
			for (size_t i = 0; i < size; ++i) {
				AttributePtr attr = attributes[i];
				if (!attr->isValid()) {
					return false;
				}
			}
			return true;
		}

		bool TransactionChecker::checkTransactionProgram(const TransactionPtr &transaction) {
			const std::vector<ProgramPtr> programs = transaction->getPrograms();
			size_t size = programs.size();
			for (size_t i = 0; i < size; ++i) {
				if (!programs[i]->isValid()) {
					return false;
				}
			}

			return true;
		}

		bool TransactionChecker::checkTransactionPayload(const TransactionPtr &transaction) {
			const PayloadPtr payloadPtr = transaction->getPayload();
			return payloadPtr->isValid();
		}
	}
}