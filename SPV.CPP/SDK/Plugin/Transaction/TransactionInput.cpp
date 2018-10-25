// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include "TransactionInput.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionInput::TransactionInput() :
				_txHash(UINT256_ZERO),
				_index(0),
				_sequence(0) {
		}

		TransactionInput::TransactionInput(const UInt256 &txHash, uint32_t index) :
				_txHash(txHash),
				_index(index),
				_sequence(0) {

		}

		TransactionInput::~TransactionInput() {

		}

		const UInt256 &TransactionInput::getTransctionHash() const {
			return _txHash;
		}

		void TransactionInput::setTransactionHash(const UInt256 &hash) {
			_txHash = hash;
		}

		uint32_t TransactionInput::getIndex() const {
			return _index;
		}

		void TransactionInput::setIndex(uint32_t index) {
			_index = index;
		}

		uint32_t TransactionInput::getSequence() const {
			return _sequence;
		}

		void TransactionInput::setSequence(uint32_t sequence) {
			_sequence = sequence;
		}

		size_t TransactionInput::getSize() const {
			return sizeof(_txHash) + sizeof(_index) + sizeof(_sequence);
		}

		void TransactionInput::Serialize(ByteStream &ostream) const {
			ostream.writeBytes(_txHash.u8, sizeof(UInt256));
			ostream.writeUint16(uint16_t(_index));
			ostream.writeUint32(_sequence);
		}

		bool TransactionInput::Deserialize(ByteStream &istream) {
			if (!istream.readBytes(_txHash.u8, sizeof(_txHash))) {
				Log::error("deserialize tx's txHash error");
				return false;
			}

			uint16_t index;
			if (!istream.readUint16(index)) {
				Log::error("deserialize tx index error");
				return false;
			}
			_index = index;

			if (!istream.readUint32(_sequence)) {
				Log::error("deserialize tx sequence error");
				return false;
			}

			return true;
		}

		nlohmann::json TransactionInput::toJson() const {
			nlohmann::json jsonData;
			jsonData["TxHash"] = Utils::UInt256ToString(_txHash, true);
			jsonData["Index"] = _index;
			jsonData["Sequence"] = _sequence;
			return jsonData;
		}

		void TransactionInput::fromJson(const nlohmann::json &jsonData) {
			_txHash = Utils::UInt256FromString(jsonData["TxHash"].get<std::string>(), true);
			_index = jsonData["Index"].get<uint32_t>();
			_sequence = jsonData["Sequence"].get<uint32_t>();
		}

	}
}
