// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

namespace Elastos {
	namespace SDK {

		TransactionOutput::TransactionOutput(uint64_t amount, const ByteData &script) {
			_output = new BRTxOutput;
			BRTxOutputSetScript(_output, script.data, script.length);
			_output->amount = amount;
		}

		TransactionOutput::~TransactionOutput() {
			if (nullptr != _output) {
				delete _output;
			}
		}

		std::string TransactionOutput::toString() const {
			//todo complete me
			return "";
		}

		BRTxOutput *TransactionOutput::getRaw() const {
			return _output;
		}

		std::string TransactionOutput::getAddress() const {
			return _output->address;
		}

		void TransactionOutput::setAddress(std::string address) {
			BRTxOutputSetAddress(_output, address.c_str());
		}

		uint64_t TransactionOutput::getAmount() const {
			return _output->amount;
		}

		void TransactionOutput::setAmount(
			uint64_t amount) {
			_output->amount = amount;
		}

		ByteData TransactionOutput::getScript() const {
			return ByteData(_output->script, _output->scriptLen);
		}

	}
}