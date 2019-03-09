// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Crypto/Key.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadDefault.h>
#include <SDK/Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

#include <Core/BRTransaction.h>

#include <iostream>
#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_assetId(UINT256_ZERO),
				_amount(0),
				_outputLock(0),
				_programHash(UINT168_ZERO),
				_outputType(Type::Default) {
			_payload = GeneratePayload(_outputType);
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			_amount = output.GetAmount();
			_assetId = output.GetAssetId();
			_programHash = output.GetProgramHash();
			_outputLock = output.GetOutputLock();
			_outputType = output.GetType();
			_payload = GeneratePayload(_outputType);
			*_payload = *output.GetPayload();
		}

		TransactionOutput::TransactionOutput(uint64_t a, const Address &addr, const UInt256 &assetID,
											 Type type, const OutputPayloadPtr &payload) :
			_amount(a),
			_outputLock(0),
			_outputType(type) {
			_assetId = assetID;
			_programHash = addr.ProgramHash();
			if (payload == nullptr) {
				_payload = GeneratePayload(_outputType);
			} else {
				_payload = payload;
			}
		}

		TransactionOutput::TransactionOutput(uint64_t a, const UInt168 &programHash, const UInt256 &assetID,
											 Type type, const OutputPayloadPtr &payload) :
			_amount(a),
			_outputLock(0),
			_outputType(type) {
			_assetId = assetID;
			_programHash = programHash;
			if (payload == nullptr) {
				_payload = GeneratePayload(_outputType);
			} else {
				_payload = payload;
			}
		}

		TransactionOutput::~TransactionOutput() {
		}

		Address TransactionOutput::GetAddress() const {
			return Address(_programHash);
		}

		uint64_t TransactionOutput::GetAmount() const {
			return _amount;
		}

		void TransactionOutput::SetAmount(uint64_t a) {
			_amount = a;
		}

		void TransactionOutput::Serialize(ByteStream &ostream) const {
			ostream.WriteBytes(_assetId.u8, sizeof(_assetId));
			ostream.WriteUint64(_amount);
			ostream.WriteUint32(_outputLock);
			ostream.WriteBytes(_programHash.u8, sizeof(_programHash));
		}

		bool TransactionOutput::Deserialize(ByteStream &istream) {
			if (!istream.ReadBytes(_assetId.u8, sizeof(_assetId))) {
				Log::error("deserialize output assetid error");
				return false;
			}

			if (!istream.ReadUint64(_amount)) {
				Log::error("deserialize output _amount error");
				return false;
			}

			if (!istream.ReadUint32(_outputLock)) {
				Log::error("deserialize output lock error");
				return false;
			}

			if (!istream.ReadBytes(_programHash.u8, sizeof(_programHash))) {
				Log::error("deserialize output program hash error");
				return false;
			}

			return true;
		}

		void TransactionOutput::Serialize(ByteStream &ostream, uint8_t txVersion) const {
			Serialize(ostream);

			if (txVersion >= Transaction::TxVersion::V09) {
				ostream.WriteUint8(_outputType);
				_payload->Serialize(ostream);
			}
		}

		bool TransactionOutput::Deserialize(ByteStream &istream, uint8_t txVersion) {
			if (!Deserialize(istream)) {
				Log::error("tx output deserialize default part error");
				return false;
			}

			if (txVersion >= Transaction::TxVersion::V09) {
				uint8_t outputType = 0;
				if (!istream.ReadUint8(outputType)) {
					Log::error("tx output deserialize output type error");
					return false;
				}
				_outputType = static_cast<Type>(outputType);

				_payload = GeneratePayload(_outputType);

				if (!_payload->Deserialize(istream)) {
					Log::error("tx output deserialize payload error");
					return false;
				}
			}

			return true;
		}

		bool TransactionOutput::IsValid() const {
			return true;
		}

		const UInt256 &TransactionOutput::GetAssetId() const {
			return _assetId;
		}

		void TransactionOutput::SetAssetId(const UInt256 &assetId) {
			_assetId = assetId;
		}

		uint32_t TransactionOutput::GetOutputLock() const {
			return _outputLock;
		}

		void TransactionOutput::SetOutputLock(uint32_t lock) {
			_outputLock = lock;
		}

		const UInt168 &TransactionOutput::GetProgramHash() const {
			return _programHash;
		}

		void TransactionOutput::SetProgramHash(const UInt168 &hash) {
			_programHash = hash;
		}

		const TransactionOutput::Type &TransactionOutput::GetType() const {
			return _outputType;
		}

		void TransactionOutput::SetType(const Type &type) {
			_outputType = type;
		}

		const OutputPayloadPtr &TransactionOutput::GetPayload() const {
			return _payload;
		}

		OutputPayloadPtr &TransactionOutput::GetPayload() {
			return _payload;
		}

		void TransactionOutput::SetPayload(const OutputPayloadPtr &payload) {
			_payload = payload;
		}

		OutputPayloadPtr TransactionOutput::GeneratePayload(const Type &type) {
			OutputPayloadPtr payload;

			switch (type) {
				case Default:
					payload = OutputPayloadPtr(new PayloadDefault());
					break;
				case VoteOutput:
					payload = OutputPayloadPtr(new PayloadVote());
					break;

				default:
					payload = nullptr;
					break;
			}

			return payload;
		}

		nlohmann::json TransactionOutput::ToJson() const {
			nlohmann::json j;

			j["Amount"] = _amount;
			j["AssetId"] = Utils::UInt256ToString(_assetId, true);
			j["OutputLock"] = _outputLock;
			j["ProgramHash"] = Utils::EncodeHex(_programHash.u8, sizeof(_programHash));
			j["Address"] = Address(_programHash).String();
			return j;
		}

		void TransactionOutput::FromJson(const nlohmann::json &j) {
			_amount = j["Amount"].get<uint64_t>();
			_assetId = Utils::UInt256FromString(j["AssetId"].get<std::string>(), true);
			_outputLock = j["OutputLock"].get<uint32_t>();
			_programHash = Utils::UInt168FromString(j["ProgramHash"].get<std::string>());
		}

		nlohmann::json TransactionOutput::ToJson(uint8_t txVersion) const {
			nlohmann::json j = ToJson();

			if (txVersion >= Transaction::TxVersion::V09) {
				j["OutputType"] = _outputType;
				j["Payload"] = _payload->ToJson();
			}

			return j;
		}

		void TransactionOutput::FromJson(const nlohmann::json &j, uint8_t txVersion) {
			FromJson(j);

			if (txVersion >= Transaction::TxVersion::V09) {
				_outputType = j["OutputType"];
				_payload = GeneratePayload(_outputType);
				_payload->FromJson(j["Payload"]);
			}
		}

		size_t TransactionOutput::GetSize() const {
			return sizeof(_assetId) + sizeof(_amount) + sizeof(_outputLock) + sizeof(_programHash);
		}

	}
}
