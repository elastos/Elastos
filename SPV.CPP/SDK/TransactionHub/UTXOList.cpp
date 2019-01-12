// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXOList.h"

#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Common/ParamChecker.h>

#include <boost/bind.hpp>

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

		void UTXOList::AddByTxInput(const TransactionInput &input, uint64_t amount, uint32_t confirms) {
			_utxos.emplace_back(input.getTransctionHash(), input.getIndex(), amount, confirms);
		}

		void UTXOList::AddUTXO(const UInt256 &hash, uint32_t index, uint64_t amount, uint32_t confirms) {
			_utxos.emplace_back(hash, index, amount, confirms);
		}

		void UTXOList::RemoveAt(size_t index) {
			if (index < _utxos.size())
				_utxos.erase(_utxos.begin() + index);
		}

		bool UTXOList::Compare(const UTXO &o1, const UTXO &o2) const {
			return o1.amount <= o2.amount;
		}

		void UTXOList::SortBaseOnOutputAmount(uint64_t totalOutputAmount, uint64_t feePerKB) {
			uint64_t Threshold = totalOutputAmount * 2 + feePerKB;
			size_t ThresholdIndex = 0;

			std::sort(_utxos.begin(), _utxos.end(),
					  boost::bind(&UTXOList::Compare, this, _1, _2));

			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (_utxos[i].amount > Threshold) {
					ThresholdIndex = i;
					break;
				}
			}

			if (ThresholdIndex > 0)
				std::reverse(_utxos.begin(), _utxos.begin() + ThresholdIndex);
		}

		const std::vector<UTXO> &UTXOList::GetUTXOs() const {
			return _utxos;
		}

	}
}

