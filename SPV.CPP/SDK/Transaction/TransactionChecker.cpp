// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ParamChecker.h>
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
			ParamChecker::checkCondition(!checkTransactionOutput(_transaction), Error::Transaction,
										 "Transaction output error");

			ParamChecker::checkCondition(!checkTransactionAttribute(_transaction), Error::Transaction,
										 "Transaction attribute error");

			ParamChecker::checkCondition(!checkTransactionProgram(_transaction), Error::Transaction,
										 "Transaction program error");

			ParamChecker::checkCondition(!checkTransactionPayload(_transaction), Error::Transaction,
										 "Transaction payload error");
		}

		bool TransactionChecker::checkTransactionOutput(const TransactionPtr &transaction) {


			const std::vector<TransactionOutput> &outputs = transaction->getOutputs();
			ParamChecker::checkCondition(outputs.size() < 1, Error::Transaction, "Tx without output");

			bool hasChange = false;
			bool hasOutput = false;
			std::string toAddress = outputs[0].getAddress();

			std::vector<std::string> addresses = _wallet->getAllAddresses();
			for (size_t i = 0; i < outputs.size(); ++i) {
				const TransactionOutput &output = outputs[i];
				ParamChecker::checkCondition(!Address::UInt168IsValid(output.getProgramHash()), Error::Transaction,
											 "Tx output's program hash is not valid");
				if (std::find(addresses.begin(), addresses.end(), output.getAddress()) != addresses.end()) {
//					if (hasChange) //should have only one change output per tx
//						return false;
					hasChange = true;
				} else {
					if (hasOutput) //todo we support only one output, modify this if we support multi-output later
						SPDLOG_TRACE(Log::getLogger(), "Transaction outputs have multiple outcoming output.");
//						return false;
//					if (!Address::isValidProgramHash(output->getProgramHash(), transaction->getTransactionType())) {
//						return false;
//					}
					hasOutput = true;
				}
			}

			if (hasChange)
				SPDLOG_TRACE(Log::getLogger(), "Transaction outputs have multiple change output.");
			if (!hasOutput)
				SPDLOG_TRACE(Log::getLogger(), "Transaction has no outcoming output.");

			return true;
		}

		bool TransactionChecker::checkTransactionAttribute(const TransactionPtr &transaction) {
			const std::vector<Attribute> &attributes = transaction->getAttributes();
			size_t size = attributes.size();
			for (size_t i = 0; i < size; ++i) {
				if (!attributes[i].isValid()) {
					return false;
				}
			}
			return true;
		}

		bool TransactionChecker::checkTransactionProgram(const TransactionPtr &transaction) {
			const std::vector<Program> &programs = transaction->getPrograms();
			size_t size = programs.size();
			for (size_t i = 0; i < size; ++i) {
				if (!programs[i].isValid(transaction.get())) {
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