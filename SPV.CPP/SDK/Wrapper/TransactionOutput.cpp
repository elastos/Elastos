// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
#include <iostream>
#include "TransactionOutput.h"

namespace Elastos {
	namespace SDK {

		TransactionOutput::TransactionOutput() {

			_output = boost::shared_ptr<BRTxOutput>(new BRTxOutput);
		}

		TransactionOutput::TransactionOutput(BRTxOutput *output) {

			_output = boost::shared_ptr<BRTxOutput>(output);
		}

		TransactionOutput::TransactionOutput(uint64_t amount, const ByteData &script) {
			_output = boost::shared_ptr<BRTxOutput>(new BRTxOutput);
			_output.get()->script = nullptr;
			BRTxOutputSetScript(_output.get(), script.data, script.length);
			_output->amount = amount;
		}

		std::string TransactionOutput::toString() const {
			//todo complete me
			return "";
		}

		BRTxOutput *TransactionOutput::getRaw() const {
			return _output.get();
		}

		std::string TransactionOutput::getAddress() const {
			return _output->address;
		}

		void TransactionOutput::setAddress(std::string address) {
			BRTxOutputSetAddress(_output.get(), address.c_str());
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

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			uint8_t assetIdData[256 / 8];
			UInt256Set(assetIdData, _assetId);
			ostream.putBytes(assetIdData, 256 / 8);

			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, _output->amount);
			ostream.putBytes(amountData, 64 / 8);

			uint8_t outputLockData[32 / 8];
			UInt32SetLE(outputLockData, _outputLock);
			ostream.putBytes(outputLockData, 32 / 8);

			uint8_t programHashData[168 / 8];
			UInt168Set(programHashData, _programHash);
			ostream.putBytes(programHashData, 168 / 8);
		}

		void TransactionOutput::Deserialize(ByteStream &istream) {
			uint8_t assetIdData[256 / 8];
			istream.getBytes(assetIdData, 256 / 8);
			UInt256Get(&_assetId, assetIdData);

			uint8_t amountData[64 / 8];
			istream.getBytes(amountData, 64 / 8);
			_output->amount = UInt64GetLE(amountData);

			uint8_t outputLockData[32 / 8];
			istream.getBytes(outputLockData, 32 / 8);
			_outputLock = UInt32GetLE(outputLockData);

			uint8_t programHashData[168 / 8];
			istream.getBytes(programHashData, 168 / 8);
			UInt168Get(&_programHash, programHashData);
		}

		const UInt256 &TransactionOutput::getAssetId() const {
			return _assetId;
		}

		void TransactionOutput::setAssetId(const UInt256 &assetId) {
			_assetId = assetId;
		}

		uint32_t TransactionOutput::getOutputLock() const {
			return _outputLock;
		}

		void TransactionOutput::setOutputLock(uint32_t outputLock) {
			_outputLock = outputLock;
		}

		const UInt168 &TransactionOutput::getProgramHash() const {
			return _programHash;
		}

		void TransactionOutput::setProgramHash(const UInt168 &hash) {
			_programHash = hash;
		}

	}
}