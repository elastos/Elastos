// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <cstring>
#include <Core/BRTransaction.h>

#include "Core/BRTransaction.h"
#include "ELATxOutput.h"
#include "TransactionOutput.h"
#include "Utils.h"
#include "Log.h"
#include "Key.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() {
			_output = ELATxOutputNew();
		}

		TransactionOutput::TransactionOutput(ELATxOutput *output) :
			_output(output) {
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			_output = ELATxOutputNew();
			CMBlock script = output.getScript();
			BRTxOutputSetScript(&_output->raw, (const uint8_t *)script, script.GetSize());
			_output->raw.amount = output.getAmount();
			_output->assetId = output.getAssetId();
			_output->programHash = output.getProgramHash();
			_output->outputLock = output.getOutputLock();
		}

		TransactionOutput::TransactionOutput(uint64_t amount, const CMBlock &script) {
			_output = ELATxOutputNew();
			BRTxOutputSetScript(&_output->raw, (const uint8_t *)script, script.GetSize());
			_output->raw.amount = amount;
			_output->assetId = Key::getSystemAssetId();
			if (!Utils::UInt168FromAddress(_output->programHash, _output->raw.address)) {
				Log::getLogger()->error("Invalid receiver address: {}", _output->raw.address);
			}
		}

		TransactionOutput::~TransactionOutput() {
			if (_output) {
				ELATxOutputFree(_output);
			}
		}

		std::string TransactionOutput::toString() const {
			//todo complete me
			return "";
		}

		BRTxOutput *TransactionOutput::getRaw() const {
			return &_output->raw;
		}

		std::string TransactionOutput::getAddress() const {
			return _output->raw.address;
		}

		void TransactionOutput::setAddress(const std::string &address) {
			BRTxOutputSetAddress(&_output->raw, address.c_str());
		}

		uint64_t TransactionOutput::getAmount() const {
			return _output->raw.amount;
		}

		void TransactionOutput::setAmount(uint64_t amount) {
			_output->raw.amount = amount;
		}

		CMBlock TransactionOutput::getScript() const {
			CMBlock data(_output->raw.scriptLen);
			memcpy(data, _output->raw.script, _output->raw.scriptLen);
			return data;
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			uint8_t assetIdData[256 / 8];
			UInt256Set(assetIdData, _output->assetId);
			ostream.putBytes(assetIdData, 256 / 8);

			uint8_t amountData[64 / 8];
			UInt64SetLE(amountData, _output->raw.amount);
			ostream.putBytes(amountData, 64 / 8);

			uint8_t outputLockData[32 / 8];
			UInt32SetLE(outputLockData, _output->outputLock);
			ostream.putBytes(outputLockData, 32 / 8);

			uint8_t programHashData[168 / 8];
			UInt168Set(programHashData, _output->programHash);
			ostream.putBytes(programHashData, 168 / 8);
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			uint8_t assetIdData[256 / 8];
			istream.getBytes(assetIdData, 256 / 8);
			UInt256Get(&_output->assetId, assetIdData);

			uint8_t amountData[64 / 8];
			istream.getBytes(amountData, 64 / 8);
			_output->raw.amount = UInt64GetLE(amountData);

			uint8_t outputLockData[32 / 8];
			istream.getBytes(outputLockData, 32 / 8);
			_output->outputLock = UInt32GetLE(outputLockData);

			uint8_t programHashData[168 / 8];
			istream.getBytes(programHashData, 168 / 8);
			UInt168Get(&_output->programHash, programHashData);

			setAddress(Utils::UInt168ToAddress(_output->programHash));

			return true;
		}

		const UInt256 &TransactionOutput::getAssetId() const {
			return _output->assetId;
		}

		void TransactionOutput::setAssetId(const UInt256 &assetId) {
			_output->assetId = assetId;
		}

		uint32_t TransactionOutput::getOutputLock() const {
			return _output->outputLock;
		}

		void TransactionOutput::setOutputLock(uint32_t outputLock) {
			_output->outputLock = outputLock;
		}

		const UInt168 &TransactionOutput::getProgramHash() const {
			return _output->programHash;
		}

		void TransactionOutput::setProgramHash(const UInt168 &hash) {
			_output->programHash = hash;
		}

		nlohmann::json TransactionOutput::toJson() {
			nlohmann::json jsonData;

			std::string addr = _output->raw.address;
			jsonData["address"] = addr;

			jsonData["amount"] = _output->raw.amount;

			jsonData["scriptLen"] = _output->raw.scriptLen;

			jsonData["script"] = Utils::encodeHex((const uint8_t *)_output->raw.script, _output->raw.scriptLen);

			jsonData["assetId"] = Utils::UInt256ToString(_output->assetId);

			jsonData["outputLock"] = _output->outputLock;

			jsonData["programHash"] = Utils::UInt168ToString(_output->programHash);

			return jsonData;
		}

		void TransactionOutput::fromJson(nlohmann::json jsonData) {
			std::string address = jsonData["address"].get<std::string>();
			size_t addressSize = sizeof(_output->raw.address);
			strncpy(_output->raw.address, address.c_str(), addressSize - 1);
			_output->raw.address[addressSize - 1] = 0;

			_output->raw.amount = jsonData["amount"].get<uint64_t>();

			size_t scriptLen = jsonData["scriptLen"].get<size_t>();
			std::string scriptString = jsonData["script"].get<std::string>();
			BRTxOutputSetScript(&_output->raw, nullptr, 0);
			if (scriptLen > 0) {
				if (scriptLen == scriptString.length() / 2) {
					CMBlock script = Utils::decodeHex(scriptString);
					BRTxOutputSetScript(&_output->raw, (const uint8_t *)script, script.GetSize());
				} else {
					Log::getLogger()->error("scriptLen={} and script=\"{}\" do not match of json",
											_output->raw.scriptLen, scriptString);
				}
			}

			_output->assetId = Utils::UInt256FromString(jsonData["assetId"].get<std::string>());
			_output->outputLock = jsonData["outputLock"].get<uint32_t>();
			_output->programHash = Utils::UInt168FromString(jsonData["programHash"].get<std::string>());
		}

	}
}