// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionInput.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionInput::TransactionInput() :
				_index(0),
				_sequence(0) {
		}

		TransactionInput::TransactionInput(const uint256 &txHash, uint32_t index) :
				_txHash(txHash),
				_index(index),
				_sequence(0) {

		}

		TransactionInput::~TransactionInput() {

		}

		const uint256 &TransactionInput::GetTransctionHash() const {
			return _txHash;
		}

		void TransactionInput::SetTransactionHash(const uint256 &hash) {
			_txHash = hash;
		}

		uint32_t TransactionInput::GetIndex() const {
			return _index;
		}

		void TransactionInput::SetIndex(uint32_t index) {
			_index = index;
		}

		uint32_t TransactionInput::GetSequence() const {
			return _sequence;
		}

		void TransactionInput::SetSequence(uint32_t sequence) {
			_sequence = sequence;
		}

		size_t TransactionInput::GetSize() const {
			return _txHash.size() + sizeof(_index) + sizeof(_sequence);
		}

		void TransactionInput::Serialize(ByteStream &ostream) const {
			ostream.WriteBytes(_txHash);
			ostream.WriteUint16(uint16_t(_index));
			ostream.WriteUint32(_sequence);
		}

		bool TransactionInput::Deserialize(const ByteStream &istream) {
			if (!istream.ReadBytes(_txHash)) {
				Log::error("deserialize tx's txHash error");
				return false;
			}

			uint16_t index;
			if (!istream.ReadUint16(index)) {
				Log::error("deserialize tx index error");
				return false;
			}
			_index = index;

			if (!istream.ReadUint32(_sequence)) {
				Log::error("deserialize tx sequence error");
				return false;
			}

			return true;
		}

		nlohmann::json TransactionInput::ToJson() const {
			nlohmann::json jsonData;
			jsonData["TxHash"] = _txHash.GetHex();
			jsonData["Index"] = _index;
			jsonData["Sequence"] = _sequence;
			return jsonData;
		}

		void TransactionInput::FromJson(const nlohmann::json &jsonData) {
			_txHash = uint256(jsonData["TxHash"].get<std::string>());
			_index = jsonData["Index"].get<uint32_t>();
			_sequence = jsonData["Sequence"].get<uint32_t>();
		}

	}
}
