// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <iostream>
#include <cstring>
#include <Core/BRTransaction.h>
#include <SDK/Wrapper/Address.h>

#include "TransactionOutput.h"
#include "Utils.h"
#include "Log.h"
#include "Key.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_assetId(UINT256_ZERO),
				_amount(0),
				_outputLock(0),
				_programHash(UINT168_ZERO) {
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			_amount = output.getAmount();
			_assetId = output.getAssetId();
			_programHash = output.getProgramHash();
			_outputLock = output.getOutputLock();
		}

		TransactionOutput::TransactionOutput(uint64_t a, const std::string &addr) :
			_amount(a),
			_outputLock(0) {
			_assetId = Key::getSystemAssetId();
			Utils::UInt168FromAddress(_programHash, addr);
		}

		TransactionOutput::TransactionOutput(uint64_t a, const UInt168 &programHash) :
			_amount(a),
			_outputLock(0) {
			_assetId = Key::getSystemAssetId();
			memcpy(_programHash.u8, programHash.u8, sizeof(_programHash.u8));
		}

		TransactionOutput::~TransactionOutput() {
		}

		std::string TransactionOutput::getAddress() const {
			return Utils::UInt168ToAddress(_programHash);
		}

		uint64_t TransactionOutput::getAmount() const {
			return _amount;
		}

		void TransactionOutput::setAmount(uint64_t a) {
			_amount = a;
		}

		size_t TransactionOutput::getSize() const {
			ByteStream stream;
			Serialize(stream);
			return stream.getBuffer().GetSize();
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_assetId.u8, sizeof(_assetId));
			ostream.writeUint64(_amount);
			ostream.writeUint32(_outputLock);
			ostream.writeBytes(_programHash.u8, sizeof(_programHash));
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_assetId.u8, sizeof(_assetId))) {
				Log::getLogger()->error("deserialize output assetid error");
				return false;
			}

			if (!istream.readUint64(_amount)) {
				Log::getLogger()->error("deserialize output _amount error");
				return false;
			}

			if (!istream.readUint32(_outputLock)) {
				Log::getLogger()->error("deserialize output lock error");
				return false;
			}

			if (!istream.readBytes(_programHash.u8, sizeof(_programHash))) {
				Log::getLogger()->error("deserialize output program hash error");
				return false;
			}

			return true;
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

		void TransactionOutput::setOutputLock(uint32_t lock) {
			_outputLock = lock;
		}

		const UInt168 &TransactionOutput::getProgramHash() const {
			return _programHash;
		}

		void TransactionOutput::setProgramHash(const UInt168 &hash) {
			_programHash = hash;
		}

		nlohmann::json TransactionOutput::toJson() const {
			nlohmann::json jsonData;

			jsonData["Amount"] = _amount;
			jsonData["AssetId"] = Utils::UInt256ToString(_assetId);
			jsonData["OutputLock"] = _outputLock;
			jsonData["ProgramHash"] = Utils::UInt168ToString(_programHash);
			return jsonData;
		}

		void TransactionOutput::fromJson(const nlohmann::json &jsonData) {
			_amount = jsonData["Amount"].get<uint64_t>();
			_assetId = Utils::UInt256FromString(jsonData["AssetId"].get<std::string>());
			_outputLock = jsonData["OutputLock"].get<uint32_t>();
			_programHash = Utils::UInt168FromString(jsonData["ProgramHash"].get<std::string>());
		}

		size_t TransactionOutput::GetSize() const {
			return sizeof(_assetId) + sizeof(_amount) + sizeof(_outputLock) + sizeof(_programHash);
		}

	}
}