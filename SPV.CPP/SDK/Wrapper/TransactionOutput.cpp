// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRTransaction.h>
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

		void TransactionOutput::Serialize(std::istream &istream) const {
			uint8_t assetIdData[256 / 8];
			UInt256Set(assetIdData, _assetId);
			istream >> assetIdData;

			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, _output->amount);
			istream >> amountData;

			uint8_t outputLockData[32 / 8];
			UInt32SetLE(outputLockData, _outputLock);
			istream >> outputLockData;

			uint8_t programHashData[168 / 8];
			UInt168Set(programHashData, _programHash);
			istream >> programHashData;
		}

		void TransactionOutput::Deserialize(std::ostream &ostream) {
			uint8_t assetIdData[256 / 8];
			ostream << assetIdData;
			UInt256Get(&_assetId, assetIdData);

			uint8_t amountData[64 / 8];
			ostream << amountData;
			_output->amount = UInt64GetLE(amountData);

			uint8_t outputLockData[32 / 8];
			ostream << outputLockData;
			_outputLock = UInt32GetLE(outputLockData);

			uint8_t programHashData[168 / 8];
			ostream << programHashData;
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