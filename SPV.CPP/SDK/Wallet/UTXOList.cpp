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

#define TX_UNCONFIRMED INT32_MAX

		UTXO::UTXO() : _n(0) {
		}

		UTXO::UTXO(const uint256 &h, uint16_t i, const BigInt &a) :
			_hash(h),
			_n(i) {
			_amount = a;
		}

		UTXO::~UTXO() {
		}

		bool UTXO::operator<(const UTXO &otherUtxo) {
			return _hash < otherUtxo._hash && _n < otherUtxo._n;
		}

		const uint256 &UTXO::Hash() const {
			return _hash;
		}

		void UTXO::SetHash(const uint256 &hash) {
			_hash = hash;
		}

		const uint16_t &UTXO::Index() const {
			return _n;
		}

		void UTXO::SetIndex(uint16_t index) {
			_n = index;
		}

		const BigInt &UTXO::Amount() const {
			return _amount;
		}

		void UTXO::SetAmount(const BigInt &amount) {
			_amount = amount;
		}

		CoinBaseUTXO::CoinBaseUTXO() : _timestamp(0), _blockHeight(0) {
		}

		CoinBaseUTXO::~CoinBaseUTXO() {
		}

		const uint256 &CoinBaseUTXO::AssetID() const {
			return _assetID;
		}

		void CoinBaseUTXO::SetAssetID(const uint256 &assetID) {
			_assetID = assetID;
		}

		const uint168 &CoinBaseUTXO::ProgramHash() const {
			return _programHash;
		}

		void CoinBaseUTXO::SetProgramHash(const uint168 &programHash) {
			_programHash = programHash;
		}

		const uint32_t &CoinBaseUTXO::OutputLock() const {
			return _outputLock;
		}

		void CoinBaseUTXO::SetOutputLock(uint32_t outputLock) {
			_outputLock = outputLock;
		}

		const uint32_t &CoinBaseUTXO::BlockHeight() const {
			return _blockHeight;
		}

		void CoinBaseUTXO::SetBlockHeight(uint32_t height) {
			_blockHeight = height;
		}

		const time_t &CoinBaseUTXO::Timestamp() const {
			return _timestamp;
		}

		void CoinBaseUTXO::SetTimestamp(time_t t) {
			_timestamp = t;
		}

		bool CoinBaseUTXO::Spent() const {
			return _spent;
		}

		void CoinBaseUTXO::SetSpent(bool status) {
			_spent = status;
		}

		uint32_t CoinBaseUTXO::GetConfirms(uint32_t lastBlockHeight) const {
			if (_blockHeight == TX_UNCONFIRMED)
				return 0;

			return lastBlockHeight >= _blockHeight ? lastBlockHeight - _blockHeight + 1 : 0;
		}

		bool UTXOList::Contains(const UTXO &o) const {
			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (o._hash == _utxos[i].Hash() && _utxos[i].Index() == o.Index()) {
					return true;
				}
			}

			return false;
		}

		bool UTXOList::Contains(const uint256 &hash, uint32_t n) const {
			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (hash == _utxos[i]._hash && _utxos[i]._n == n) {
					return true;
				}
			}

			return false;
		}

		bool UTXOList::Contains(const TransactionInput &input) const {
			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (input.GetTransctionHash() == _utxos[i]._hash && _utxos[i]._n == input.GetIndex())
					return true;
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

		void UTXOList::AddByTxInput(const TransactionInput &input, const BigInt &amount) {
			_utxos.emplace_back(input.GetTransctionHash(), input.GetIndex(), amount);
		}

		void UTXOList::AddUTXO(const UTXO &o) {
			_utxos.push_back(o);
		}

		void UTXOList::RemoveAt(size_t index) {
			if (index < _utxos.size())
				_utxos.erase(_utxos.begin() + index);
		}

		bool UTXOList::Compare(const UTXO &o1, const UTXO &o2) const {
			return o1._amount <= o2._amount;
		}

		void UTXOList::SortBaseOnOutputAmount(const BigInt &totalOutputAmount, uint64_t feePerKB) {
			BigInt Threshold = totalOutputAmount * 2 + feePerKB;
			size_t ThresholdIndex = 0;

			std::sort(_utxos.begin(), _utxos.end(),
					  boost::bind(&UTXOList::Compare, this, _1, _2));

			for (size_t i = 0; i < _utxos.size(); ++i) {
				if (_utxos[i]._amount > Threshold) {
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

