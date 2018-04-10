// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRTransaction.h"
#include "TransactionInput.h"

namespace Elastos {
	namespace SDK {

		TransactionInput::TransactionInput(UInt256 hash, uint64_t index, uint64_t amount, ByteData script,
										   ByteData signature, uint64_t sequence) {
			_input = new BRTxInput;
			_input->txHash = hash;
			_input->index = index;
			_input->amount = amount;
			BRTxInputSetScript(_input, script.data, script.length);
			BRTxInputSetSignature(_input, signature.data, signature.length);
			_input->sequence = sequence;
		}

		TransactionInput::~TransactionInput() {
			if (nullptr != _input) {
				free(_input);
			}
		}

		std::string TransactionInput::toString() const {
			//todo complete me
			return "";
		}

		BRTxInput *TransactionInput::getRaw() const {
			return _input;
		}

		std::string TransactionInput::getAddress() const {
			return _input->address;
		}

		void TransactionInput::setAddress(std::string address) {
			BRTxInputSetAddress(_input, address.c_str());
		}

		UInt256 TransactionInput::getHash() const {
			return _input->txHash;
		}

		uint32_t TransactionInput::getIndex() const {
			return _input->index;
		}

		uint64_t TransactionInput::getAmount() const {
			return _input->amount;
		}

		ByteData TransactionInput::getScript() const {
			return ByteData(_input->script, _input->scriptLen);
		}

		ByteData TransactionInput::getSignature() const {
			return ByteData(_input->signature, _input->sigLen);
		}

		uint32_t TransactionInput::getSequence() const {
			return _input->sequence;
		}

	}
}
