// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <WalletCore/Key.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadDefault.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>

#include <iostream>
#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		TransactionOutput::TransactionOutput() :
				_fixedIndex(0),
				_outputLock(0),
				_outputType(Type::Default) {
			_addr = AddressPtr(new Address());
			_amount.setUint64(0);
			_payload = GeneratePayload(_outputType);
		}

		TransactionOutput::TransactionOutput(const TransactionOutput &output) {
			_addr = AddressPtr(new Address());
			this->operator=(output);
		}

		TransactionOutput &TransactionOutput::operator=(const TransactionOutput &o) {
			_fixedIndex = o._fixedIndex;
			_amount = o._amount;
			_assetID = o._assetID;
			*_addr = *o._addr;
			_outputLock = o._outputLock;
			_outputType = o._outputType;
			_payload = GeneratePayload(o._outputType);
			*_payload = *o._payload;
			return *this;
		}

		TransactionOutput::TransactionOutput(const BigInt &a, const Address &addr, const uint256 &assetID,
											 Type type, const OutputPayloadPtr &payload) :
			_fixedIndex(0),
			_outputLock(0),
			_outputType(type) {

			_assetID = assetID;
			_amount = a;
			_addr = AddressPtr(new Address(addr));

			if (payload == nullptr) {
				_payload = GeneratePayload(_outputType);
			} else {
				_payload = payload;
			}
		}

		TransactionOutput::~TransactionOutput() {
		}

		const AddressPtr &TransactionOutput::Addr() const {
			return _addr;
		}

		const BigInt &TransactionOutput::Amount() const {
			return _amount;
		}

		void TransactionOutput::SetAmount(const BigInt &a) {
			_amount = a;
		}

		size_t TransactionOutput::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += _assetID.size();
			if (_assetID == Asset::GetELAAssetID()) {
				size += sizeof(uint64_t);
			} else {
				bytes_t amountBytes = _amount.getHexBytes();
				size += stream.WriteVarUint(amountBytes.size());
				size += amountBytes.size();
			}

			size += sizeof(_outputLock);
			size += _addr->ProgramHash().size();

			return size;
		}

		void TransactionOutput::Serialize(ByteStream &ostream, uint8_t txVersion, bool extend) const {
			ostream.WriteBytes(_assetID);

			if (_assetID == Asset::GetELAAssetID()) {
				bytes_t bytes = _amount.getHexBytes(true);
				uint64_t amount = 0;
				memcpy(&amount, &bytes[0], MIN(bytes.size(), sizeof(uint64_t)));
				ostream.WriteUint64(amount);
			} else {
				ostream.WriteVarBytes(_amount.getHexBytes());
			}

			ostream.WriteUint32(_outputLock);
			ostream.WriteBytes(_addr->ProgramHash());

			if (txVersion >= Transaction::TxVersion::V09) {
				ostream.WriteUint8(_outputType);
				_payload->Serialize(ostream);
			}

			if (extend) {
				ostream.WriteUint16(_fixedIndex);
			}
		}

		bool TransactionOutput::Deserialize(const ByteStream &istream, uint8_t txVersion, bool extend) {
			if (!istream.ReadBytes(_assetID)) {
				Log::error("deserialize output assetid error");
				return false;
			}

			if (_assetID == Asset::GetELAAssetID()) {
				uint64_t amount;
				if (!istream.ReadUint64(amount)) {
					Log::error("deserialize output amount error");
					return false;
				}
				_amount.setHexBytes(bytes_t(&amount, sizeof(amount)), true);
			} else {
				bytes_t bytes;
				if (!istream.ReadVarBytes(bytes)) {
					Log::error("deserialize output BN amount error");
					return false;
				}
				_amount.setHexBytes(bytes);
			}

			if (!istream.ReadUint32(_outputLock)) {
				Log::error("deserialize output lock error");
				return false;
			}

			uint168 programHash;
			if (!istream.ReadBytes(programHash)) {
				Log::error("deserialize output program hash error");
				return false;
			}
			_addr->SetProgramHash(programHash);

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

			if (extend && !istream.ReadUint16(_fixedIndex)) {
				Log::error("deserialize output index error");
				return false;
			}

			return true;
		}

		bool TransactionOutput::IsValid() const {
			return true;
		}

		const uint256 &TransactionOutput::AssetID() const {
			return _assetID;
		}

		void TransactionOutput::SetAssetID(const uint256 &assetId) {
			_assetID = assetId;
		}

		uint32_t TransactionOutput::OutputLock() const {
			return _outputLock;
		}

		void TransactionOutput::SetOutputLock(uint32_t lock) {
			_outputLock = lock;
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

			j["FixedIndex"] = _fixedIndex;
			j["Amount"] = _amount.getDec();
			j["AssetId"] = _assetID.GetHex();
			j["OutputLock"] = _outputLock;
			j["ProgramHash"] = _addr->ProgramHash().GetHex();
			j["Address"] = _addr->String();
			j["OutputType"] = _outputType;
			j["Payload"] = _payload->ToJson();

			return j;
		}

		void TransactionOutput::FromJson(const nlohmann::json &j) {
			_fixedIndex = j["FixedIndex"].get<uint16_t>();
			if (j["Amount"].is_number()) {
				_amount.setUint64(j["Amount"].get<uint64_t>());
			} else if (j["Amount"].is_string()) {
				_amount.setDec(j["Amount"].get<std::string>());
			}
			_assetID.SetHex(j["AssetId"].get<std::string>());
			_outputLock = j["OutputLock"].get<uint32_t>();
			uint168 programHash;
			programHash.SetHex(j["ProgramHash"].get<std::string>());
			_addr->SetProgramHash(programHash);

			_outputType = Type(j["OutputType"].get<uint8_t>());
			_payload = GeneratePayload(_outputType);
			_payload->FromJson(j["Payload"]);
		}

		size_t TransactionOutput::GetSize() const {
			return _assetID.size() + sizeof(_amount) + sizeof(_outputLock) + _addr->ProgramHash().size();
		}

		uint16_t TransactionOutput::FixedIndex() const {
			return _fixedIndex;
		}

		void TransactionOutput::SetFixedIndex(uint16_t index) {
			_fixedIndex = index;
		}

	}
}
