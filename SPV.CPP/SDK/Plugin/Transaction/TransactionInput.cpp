// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionInput.h"

#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionInput::TransactionInput() :
			_index(0),
			_sequence(0),
			_containDetail(false) {
		}

		TransactionInput::TransactionInput(const TransactionInput &input) {
			this->operator=(input);
		}

		TransactionInput &TransactionInput::operator=(const TransactionInput &input) {
			_txHash = input._txHash;
			_index = input._index;
			_sequence = input._sequence;

			_containDetail = input._containDetail;
			_amount = input._amount;
			_addr = input._addr;

			return *this;
		}

		TransactionInput::TransactionInput(const uint256 &txHash, uint16_t index) :
			_txHash(txHash),
			_index(index),
			_sequence(0),
			_containDetail(false) {

		}

		TransactionInput::TransactionInput(const uint256 &txHash, uint16_t index, const BigInt &amount, const Address &addr) :
			_txHash(txHash),
			_index(index),
			_sequence(0),
			_containDetail(true),
			_amount(amount),
			_addr(addr) {

		}

		TransactionInput::~TransactionInput() {

		}

		const uint256 &TransactionInput::TxHash() const {
			return _txHash;
		}

		void TransactionInput::SetTxHash(const uint256 &hash) {
			_txHash = hash;
		}

		uint16_t TransactionInput::Index() const {
			return _index;
		}

		void TransactionInput::SetIndex(uint16_t index) {
			_index = index;
		}

		uint32_t TransactionInput::Sequence() const {
			return _sequence;
		}

		void TransactionInput::SetSequence(uint32_t sequence) {
			_sequence = sequence;
		}

		size_t TransactionInput::GetSize() const {
			return _txHash.size() + sizeof(_index) + sizeof(_sequence);
		}

		void TransactionInput::FixDetail(const BigInt &amount, const Address &addr) {
			_containDetail = true;
			_amount = amount;
			_addr = addr;
		}

		bool TransactionInput::ContainDetail() const {
			return _addr.Valid();
		}

		const BigInt &TransactionInput::GetAmount() const {
			return _amount;
		}

		void TransactionInput::SetAmount(const BigInt &amount) {
			_amount = amount;
		}

		const Address &TransactionInput::GetAddress() const {
			return _addr;
		}

		void TransactionInput::SetAddress(const Address &addr) {
			_addr = addr;
		}

		bool TransactionInput::operator==(const TransactionInput &in) const {
			if (_containDetail != in._containDetail)
				return false;

			bool equal = _txHash == in._txHash &&
						 _index == in._index &&
						 _sequence == in._sequence;

			if (_containDetail)
				equal = equal && _amount == in._amount && _addr == in._addr;

			return equal;
		}

		bool TransactionInput::operator!=(const TransactionInput &in) const {
			return !operator==(in);
		}

		size_t TransactionInput::EstimateSize() const {
			size_t size = 0;

			size += _txHash.size();
			size += sizeof(_index);
			size += sizeof(_sequence);

			return size;
		}

		void TransactionInput::Serialize(ByteStream &stream, bool extend) const {
			stream.WriteBytes(_txHash);
			stream.WriteUint16(_index);
			stream.WriteUint32(_sequence);
			if (extend) {
				stream.WriteUint8(_containDetail ? 1 : 0);
				if (_containDetail) {
					stream.WriteVarBytes(_amount.getHexBytes());
					stream.WriteBytes(_addr.ProgramHash());
				}
			}
		}

		bool TransactionInput::Deserialize(const ByteStream &stream, bool extend) {
			if (!stream.ReadBytes(_txHash)) {
				Log::error("deser input txHash");
				return false;
			}

			if (!stream.ReadUint16(_index)) {
				Log::error("deser input index");
				return false;
			}

			if (!stream.ReadUint32(_sequence)) {
				Log::error("deser input sequence");
				return false;
			}

			if (extend) {
				uint8_t containDetail;
				if (!stream.ReadUint8(containDetail)) {
					Log::error("deser contain detail");
					return false;
				}
				_containDetail = containDetail != 0;

				if (_containDetail) {
					bytes_t amountBytes;
					if (!stream.ReadVarBytes(amountBytes)) {
						Log::error("deser input amount");
						return false;
					}
					_amount.setHexBytes(amountBytes);

					uint168 programHash;
					if (!stream.ReadBytes(programHash)) {
						Log::error("deser input addr");
						return false;
					}
					_addr.SetProgramHash(programHash);
				}
			}

			return true;
		}

		nlohmann::json TransactionInput::ToJson() const {
			nlohmann::json j;
			j["TxHash"] = _txHash.GetHex();
			j["Index"] = _index;
			j["Sequence"] = _sequence;
			j["ContainDetail"] = _containDetail;
			j["Amount"] = _amount.getDec();
			j["Address"] = _addr.String();
			return j;
		}

		void TransactionInput::FromJson(const nlohmann::json &j) {
			_txHash = uint256(j["TxHash"].get<std::string>());
			_index = j["Index"].get<uint16_t>();
			_sequence = j["Sequence"].get<uint32_t>();
			_containDetail = j["ContainDetail"].get<bool>();
			_amount.setDec(j["Amount"].get<std::string>());
			_addr = Address(j["Address"].get<std::string>());
		}

	}
}
