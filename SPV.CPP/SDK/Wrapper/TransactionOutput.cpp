// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <SDK/Common/Utils.h>

#include "ELABRTxOutput.h"
#include "TransactionOutput.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		TransactionOutput::TransactionOutput() {

			_output = boost::shared_ptr<BRTxOutput>(new BRTxOutput);
			_output.get()->script = nullptr;
			_output.get()->scriptLen = 0;
			_assetId = UINT256_ZERO;
			_programHash = UINT168_ZERO;
		}

		TransactionOutput::TransactionOutput(BRTxOutput *output) {
			assert(output != nullptr);
			_output = boost::shared_ptr<BRTxOutput>(output);
			convertFrom(output);
		}

		TransactionOutput::TransactionOutput(uint64_t amount, const CMBlock &script) {
			_output = boost::shared_ptr<BRTxOutput>(new BRTxOutput);
			_output.get()->script = nullptr;
			BRTxOutputSetScript(_output.get(), script, script.GetSize());
			_output->amount = amount;
			_assetId = UINT256_ZERO;
			_programHash = UINT168_ZERO;
		}

		std::string TransactionOutput::toString() const {
			//todo complete me
			return "";
		}

		BRTxOutput *TransactionOutput::getRaw() const {
			return _output.get();
		}

		const std::string &TransactionOutput::getAddress() const {
			return _output->address;
		}

		void TransactionOutput::setAddress(const std::string &address) {
			BRTxOutputSetAddress(_output.get(), address.c_str());
		}

		uint64_t TransactionOutput::getAmount() const {
			return _output->amount;
		}

		void TransactionOutput::setAmount(
				uint64_t amount) {
			_output->amount = amount;
		}

		CMBlock TransactionOutput::getScript() const {
			CMBlock data(_output->scriptLen);
			memcpy(data, _output->script, _output->scriptLen);

			return data;
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

			setAddress(Utils::UInt168ToAddress(_programHash));
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

		BRTxOutput *TransactionOutput::convertToRaw() const {
			boost::shared_ptr<ELABRTxOutput> output = boost::shared_ptr<ELABRTxOutput>(new ELABRTxOutput);
			output->raw.script = nullptr;
			BRTxOutputSetScript(&output->raw, _output->script, _output->scriptLen);
			output->raw.amount = _output->amount;
			output->outputLock = _outputLock;
			UInt256Set(&output->assetId, _assetId);
			UInt168Set(&output->programHash, _programHash);
			return (BRTxOutput *) output.get();
		}

		void TransactionOutput::convertFrom(const BRTxOutput *raw) {
			ELABRTxOutput *elabrTxOutput = (ELABRTxOutput *) raw;
			UInt256Set(&_assetId, elabrTxOutput->assetId);
			UInt168Set(&_programHash, elabrTxOutput->programHash);
			_outputLock = elabrTxOutput->outputLock;
		}

		nlohmann::json TransactionOutput::toJson() {
			nlohmann::json jsonData;

			std::string addr = _output->address;
			jsonData["address"] = addr;

			jsonData["amount"] = _output->amount;

			jsonData["scriptLen"] = _output->scriptLen;
			char *script = new char[_output->scriptLen];
			memcpy(script, _output->script, _output->scriptLen);
			std::string scriptStr(script, _output->scriptLen);
			jsonData["script"] = scriptStr;

			jsonData["assetId"] = Utils::UInt256ToString(_assetId);

			jsonData["outputLock"] = _outputLock;

			jsonData["programHash"] = Utils::UInt168ToString(_programHash);

			return jsonData;
		}

		void TransactionOutput::fromJson(nlohmann::json jsonData) {
			std::string address = jsonData["address"];
			memset(_output->address, 0, sizeof(_output->address));
			memcpy(_output->address, address.data(), address.size());

			_output->amount = jsonData["amount"].get<uint64_t>();

			_output->scriptLen = jsonData["scriptLen"].get<size_t>();
			if(_output->scriptLen > 0) {
				std::string script = jsonData["script"].get<std::string>();
				memcpy(_output->script, script.c_str(), _output->scriptLen);
			}

			std::string assetID = jsonData["assetId"].get<std::string>();
			_assetId = Utils::UInt256FromString(assetID);

			_outputLock = jsonData["outputLock"].get<uint32_t>();

			std::string programHash = jsonData["programHash"].get<std::string>();
			_programHash = Utils::UInt168FromString(programHash);
		}

	}
}