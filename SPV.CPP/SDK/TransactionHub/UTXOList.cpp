// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXOList.h"

#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/BigInt.h>

#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

		bool UTXOList::Contains(const UTXO &o) const {
			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (o.hash == _utxos[i].hash && _utxos[i].n == o.n) {
					return true;
				}
			}

			return false;
		}

		bool UTXOList::Contains(const uint256 &hash, uint32_t n) const {
			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (hash == _utxos[i].hash && _utxos[i].n == n) {
					return true;
				}
			}

			return false;
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

		void UTXOList::AddByTxInput(const TransactionInput &input, const BigInt &amount, uint32_t confirms) {
			_utxos.emplace_back(input.GetTransctionHash(), input.GetIndex(), amount, confirms);
		}

		void UTXOList::AddUTXO(const UTXO &o) {
			_utxos.push_back(o);
		}

		void UTXOList::RemoveAt(size_t index) {
			if (index < _utxos.size())
				_utxos.erase(_utxos.begin() + index);
		}

		bool UTXOList::Compare(const UTXO &o1, const UTXO &o2) const {
			return o1.amount <= o2.amount;
		}

		void UTXOList::SortBaseOnOutputAmount(const BigInt &totalOutputAmount, uint64_t feePerKB) {
			BigInt Threshold = totalOutputAmount * 2 + feePerKB;
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

