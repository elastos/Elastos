// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Log.h"
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

			const std::vector<TransactionOutput *> &outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}

			bool hasChange = false;
			bool hasOutput = false;
			std::string toAddress = outputs[0]->getAddress();
			int toAddressCount = 0;

			std::vector<std::string> addresses = _wallet->getAllAddresses();
			for (size_t i = 0; i < size; ++i) {
				TransactionOutput *output = outputs[i];
				if (!Address::UInt168IsValid(output->getProgramHash())) {
					return false;
				}
				if (output->getAddress() == toAddress) {
					toAddressCount ++;
				}
				if (std::find(addresses.begin(), addresses.end(), output->getAddress()) != addresses.end()) {
//					if (hasChange) //should have only one change output per tx
//						return false;
					hasChange = true;
				} else {
					if (hasOutput) //todo we support only one output, modify this if we support multi-output later
						Log::warn("Transaction outputs have multiple outcoming output.");
//						return false;
					if (!Address::isValidProgramHash(output->getProgramHash(), transaction->getTransactionType())) {
						return false;
					}
					hasOutput = true;
				}
			}

			if (toAddressCount > 1) {
				return false;
			}

			if (hasChange)
				Log::warn("Transaction outputs have multiple change output.");
			if (!hasOutput)
				Log::warn("Transaction has no outcoming output.");

			return true;
		}

		bool TransactionChecker::checkTransactionAttribute(const TransactionPtr &transaction) {
			const std::vector<Attribute *> &attributes = transaction->getAttributes();
			size_t size = attributes.size();
			for (size_t i = 0; i < size; ++i) {
				Attribute *attr = attributes[i];
				if (!attr->isValid()) {
					return false;
				}
			}
			return true;
		}

		bool TransactionChecker::checkTransactionProgram(const TransactionPtr &transaction) {
			const std::vector<Program *> &programs = transaction->getPrograms();
			size_t size = programs.size();
			for (size_t i = 0; i < size; ++i) {
				if (!programs[i]->isValid()) {
					return false;
				}
			}

			return true;
		}

		bool TransactionChecker::checkTransactionPayload(const TransactionPtr &transaction) {
			return transaction->getPayload()->isValid();
		}
	}
}