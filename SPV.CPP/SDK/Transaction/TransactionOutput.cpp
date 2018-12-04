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
			ELATxOutputSetScript(_output, (const uint8_t *)script, script.GetSize(), output.getAddressSignType());
			_output->raw.amount = output.getAmount();
			_output->assetId = output.getAssetId();
			_output->programHash = output.getProgramHash();
			_output->outputLock = output.getOutputLock();

			std::string addr = Utils::UInt168ToAddress(_output->programHash);
			size_t addressSize = sizeof(_output->raw.address);
			strncpy(_output->raw.address, addr.c_str(), addressSize - 1);
			_output->raw.address[addressSize - 1] = 0;
		}

		TransactionOutput::TransactionOutput(const std::string &toAddress, uint64_t amount,
											 const UInt256 &assetID) {
			_output = ELATxOutputNew();

			_output->raw.amount = amount;
			if (!Utils::UInt168FromAddress(_output->programHash, toAddress)) {
				Log::getLogger()->error("Invalid to address {}", toAddress);
			}

			size_t addressSize = sizeof(_output->raw.address);
			strncpy(_output->raw.address, toAddress.c_str(), addressSize - 1);
			_output->raw.address[addressSize - 1] = 0;

			_output->assetId = assetID;
			_output->outputLock = 0;
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

		void TransactionOutput::setAddressSignType(int signType) {
			_output->signType = signType;
		}

		int TransactionOutput::getAddressSignType() const {
			return _output->signType;
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

		size_t TransactionOutput::getSize() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer().GetSize();
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_output->assetId.u8, sizeof(_output->assetId));
			ostream.writeUint64(_output->raw.amount);
			ostream.writeUint32(_output->outputLock);
			ostream.writeBytes(_output->programHash.u8, sizeof(_output->programHash));
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_output->assetId.u8, sizeof(_output->assetId))) {
				Log::getLogger()->error("deserialize output assetid error");
				return false;
			}

			if (!istream.readUint64(_output->raw.amount)) {
				Log::getLogger()->error("deserialize output amount error");
				return false;
			}

			if (!istream.readUint32(_output->outputLock)) {
				Log::getLogger()->error("deserialize output lock error");
				return false;
			}

			if (!istream.readBytes(_output->programHash.u8, sizeof(_output->programHash))) {
				Log::getLogger()->error("deserialize output program hash error");
				return false;
			}

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

		nlohmann::json TransactionOutput::toJson() const {
			nlohmann::json jsonData;

			jsonData["Amount"] = _output->raw.amount;

			jsonData["AssetId"] = Utils::UInt256ToString(_output->assetId);

			jsonData["OutputLock"] = _output->outputLock;

			jsonData["ProgramHash"] = Utils::UInt168ToString(_output->programHash);

			std::string addr = _output->raw.address;
			assert(addr == Utils::UInt168ToAddress(_output->programHash));
			jsonData["Address"] = Utils::UInt168ToAddress(_output->programHash);

			jsonData["SignType"] = _output->signType;

			return jsonData;
		}

		void TransactionOutput::fromJson(const nlohmann::json &jsonData) {
			_output->raw.amount = jsonData["Amount"].get<uint64_t>();

			_output->signType = jsonData["SignType"].get<int>();

			_output->assetId = Utils::UInt256FromString(jsonData["AssetId"].get<std::string>());
			_output->outputLock = jsonData["OutputLock"].get<uint32_t>();
			_output->programHash = Utils::UInt168FromString(jsonData["ProgramHash"].get<std::string>());

			std::string addrFromProgramHash = Utils::UInt168ToAddress(_output->programHash);
			std::string address = jsonData["Address"].get<std::string>();
			assert(address == addrFromProgramHash);
			size_t addressSize = sizeof(_output->raw.address);
			strncpy(_output->raw.address, addrFromProgramHash.c_str(), addressSize - 1);
			_output->raw.address[addressSize - 1] = 0;
		}

	}
}