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
			_sequence(0) {
		}

		TransactionInput::TransactionInput(const TransactionInput &input) {
			this->operator=(input);
		}

		TransactionInput &TransactionInput::operator=(const TransactionInput &input) {
			_txHash = input._txHash;
			_index = input._index;
			_sequence = input._sequence;

			return *this;
		}

		TransactionInput::TransactionInput(const uint256 &txHash, uint16_t index) :
			_txHash(txHash),
			_index(index),
			_sequence(0) {

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

		bool TransactionInput::operator==(const TransactionInput &in) const {
			bool equal = _txHash == in._txHash &&
						 _index == in._index &&
						 _sequence == in._sequence;

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

		void TransactionInput::Serialize(ByteStream &stream) const {
			stream.WriteBytes(_txHash);
			stream.WriteUint16(_index);
			stream.WriteUint32(_sequence);
		}

		bool TransactionInput::Deserialize(const ByteStream &stream) {
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

			return true;
		}

		nlohmann::json TransactionInput::ToJson() const {
			nlohmann::json j;
			j["TxHash"] = _txHash.GetHex();
			j["Index"] = _index;
			j["Sequence"] = _sequence;
			return j;
		}

		void TransactionInput::FromJson(const nlohmann::json &j) {
			_txHash = uint256(j["TxHash"].get<std::string>());
			_index = j["Index"].get<uint16_t>();
			_sequence = j["Sequence"].get<uint32_t>();
		}

	}
}
