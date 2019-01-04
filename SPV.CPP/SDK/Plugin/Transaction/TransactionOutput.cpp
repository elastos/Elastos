// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

#include <SDK/Base/Address.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Crypto/Key.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <Core/BRTransaction.h>

#include <iostream>
#include <cstring>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_assetId(UINT256_ZERO),
				_amount(0),
				_outputLock(0),
				_programHash(UINT168_ZERO),
				_outputType(Type::Default),
				_payloadVersion(0),
				_payload(nullptr) {
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			_amount = output.getAmount();
			_assetId = output.getAssetId();
			_programHash = output.getProgramHash();
			_outputLock = output.getOutputLock();
			_outputType = output.GetType();
			_payloadVersion = output.GetPayloadVersion();
			if (_outputType == VoteOutput) {
				_payload = PayloadPtr(new PayloadVote());
				*_payload = *output.GetPayload();
			} else {
				_payload = nullptr;
			}
		}

		TransactionOutput::TransactionOutput(uint64_t a, const std::string &addr, const UInt256 &assetID,
											 Type type, const PayloadPtr &payload) :
			_amount(a),
			_outputLock(0),
			_outputType(type),
			_payloadVersion(0) {
			_assetId = assetID;
			Utils::UInt168FromAddress(_programHash, addr);
			_payload = payload;
		}

		TransactionOutput::TransactionOutput(uint64_t a, const UInt168 &programHash, const UInt256 &assetID,
											 Type type, const PayloadPtr &payload) :
			_amount(a),
			_outputLock(0),
			_outputType(type),
			_payloadVersion(0) {
			_assetId = assetID;
			_programHash = programHash;
			_payload = payload;
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

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_assetId.u8, sizeof(_assetId));
			ostream.writeUint64(_amount);
			ostream.writeUint32(_outputLock);
			ostream.writeBytes(_programHash.u8, sizeof(_programHash));
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_assetId.u8, sizeof(_assetId))) {
				Log::error("deserialize output assetid error");
				return false;
			}

			if (!istream.readUint64(_amount)) {
				Log::error("deserialize output _amount error");
				return false;
			}

			if (!istream.readUint32(_outputLock)) {
				Log::error("deserialize output lock error");
				return false;
			}

			if (!istream.readBytes(_programHash.u8, sizeof(_programHash))) {
				Log::error("deserialize output program hash error");
				return false;
			}

			return true;
		}

		void TransactionOutput::Serialize(ByteStream &ostream, uint8_t txVersion) const {
			Serialize(ostream);

			if (txVersion >= Transaction::TxVersion::V09) {
				ostream.writeUint8(_outputType);
				ostream.writeUint8(_payloadVersion);
				_payload->Serialize(ostream, _payloadVersion);
			}
		}

		bool TransactionOutput::Deserialize(ByteStream &istream, uint8_t txVersion) {
			if (!Deserialize(istream)) {
				Log::error("tx output deserialize default part error");
				return false;
			}

			if (txVersion >= Transaction::TxVersion::V09) {
				uint8_t outputType = 0;
				if (!istream.readUint8(outputType)) {
					Log::error("tx output deserialize output type error");
					return false;
				}
				_outputType = static_cast<Type>(outputType);

				if (!istream.readUint8(_payloadVersion)) {
					Log::error("tx output deserialize payload version error");
					return false;
				}

				_payload = PayloadPtr(new PayloadVote());

				if (!_payload->Deserialize(istream, _payloadVersion)) {
					Log::error("tx output deserialize payload error");
					return false;
				}
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

		const TransactionOutput::Type &TransactionOutput::GetType() const {
			return _outputType;
		}

		void TransactionOutput::SetType(const Type &type) {
			_outputType = type;
		}

		const uint8_t &TransactionOutput::GetPayloadVersion() const {
			return _payloadVersion;
		}

		void TransactionOutput::SetPayloadVersion(const uint8_t &payloadVersion) {
			_payloadVersion = payloadVersion;
		}

		const PayloadPtr &TransactionOutput::GetPayload() const {
			return _payload;
		}

		PayloadPtr &TransactionOutput::GetPayload() {
			return _payload;
		}

		void TransactionOutput::SetPayload(const PayloadPtr &payload) {
			_payload = payload;
		}

		nlohmann::json TransactionOutput::toJson() const {
			nlohmann::json j;

			j["Amount"] = _amount;
			j["AssetId"] = Utils::UInt256ToString(_assetId, true);
			j["OutputLock"] = _outputLock;
			j["ProgramHash"] = Utils::UInt168ToString(_programHash);
			return j;
		}

		void TransactionOutput::fromJson(const nlohmann::json &j) {
			_amount = j["Amount"].get<uint64_t>();
			_assetId = Utils::UInt256FromString(j["AssetId"].get<std::string>(), true);
			_outputLock = j["OutputLock"].get<uint32_t>();
			_programHash = Utils::UInt168FromString(j["ProgramHash"].get<std::string>());
		}

		nlohmann::json TransactionOutput::toJson(uint8_t txVersion) const {
			nlohmann::json j = toJson();

			if (txVersion >= Transaction::TxVersion::V09) {
				j["OutputType"] = _outputType;
				j["PayloadVersion"] = _payloadVersion;
				j["Payload"] = _payload->toJson();
			}

			return j;
		}

		void TransactionOutput::fromJson(const nlohmann::json &j, uint8_t txVersion) {
			fromJson(j);

			if (txVersion >= Transaction::TxVersion::V09) {
				_outputType = j["OutputType"];
				_payloadVersion = j["PayloadVersion"];

				_payload = PayloadPtr(new PayloadVote());

				_payload->fromJson(j["Payload"]);
			}
		}

		size_t TransactionOutput::GetSize() const {
			return sizeof(_assetId) + sizeof(_amount) + sizeof(_outputLock) + sizeof(_programHash);
		}

	}
}
