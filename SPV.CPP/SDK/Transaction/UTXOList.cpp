// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXOList.h"
#include "TransactionInput.h"

namespace Elastos {
	namespace ElaWallet {

		bool UTXOList::Constains(const UInt256 &hash) const {
			std::vector<UTXO>::const_iterator itr = std::find_if(_utxos.begin(), _utxos.end(),
																 [&hash](const UTXO &h) {
																	 return UInt256Eq(&hash, &h.hash) == 1;
																 });
			return itr != _utxos.end();
		}

		UTXO &UTXOList::operator[](size_t i) {
			return _utxos[i];
		}

		size_t UTXOList::size() const {
			return _utxos.size();
		}

		void UTXOList::Clear() {
			_utxos.clear();
		}

		void UTXOList::AddByTxInput(const TransactionInput &input) {
			_utxos.push_back(UTXO(input.getTransctionHash(), input.getIndex()));
		}

		void UTXOList::AddUTXO(const UInt256 &hash, uint32_t index) {
			_utxos.push_back(UTXO(hash, index));
		}

		void UTXOList::RemoveAt(size_t index) {
			if (index < _utxos.size())
				_utxos.erase(_utxos.begin() + index);
		}
	}
}

