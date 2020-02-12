// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXO.h"

#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Common/ErrorChecker.h>
#include <Common/BigInt.h>

#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

#define TX_UNCONFIRMED INT32_MAX

		UTXO::UTXO() :
			_n(0),
			_timestamp(0),
			_blockHeight(TX_UNCONFIRMED),
			_output(new TransactionOutput()) {
		}

		UTXO::UTXO(const UTXO &u) :
			_output(new TransactionOutput()) {
			this->operator=(u);
		}

		UTXO &UTXO::operator=(const UTXO &u) {
			this->_n = u._n;
			this->_hash = u._hash;
			*this->_output = *u._output;
			this->_timestamp = u._timestamp;
			this->_blockHeight = u._blockHeight;
			return *this;
		}

		UTXO::UTXO(const uint256 &hash, uint16_t i, time_t t, uint32_t h, const OutputPtr &o) :
			_hash(hash),
			_n(i),
			_timestamp(t),
			_blockHeight(h),
			_output(o) {
		}

		UTXO::UTXO(const InputPtr &input) :
			_timestamp(0),
			_blockHeight(0),
			_output(nullptr) {
			_hash = input->TxHash();
			_n = input->Index();
		}

		UTXO::~UTXO() {
		}

		bool UTXO::operator==(const UTXO &u) const {
			return _hash == u._hash && _n == u._n;
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

		time_t UTXO::Timestamp() const {
			return _timestamp;
		}

		void UTXO::SetTimestamp(time_t t) {
			_timestamp = t;
		}

		uint32_t UTXO::BlockHeight() const {
			return _blockHeight;
		}

		void UTXO::SetBlockHeight(uint32_t h) {
			_blockHeight = h;
		}

		const OutputPtr &UTXO::Output() const {
			return _output;
		}

		void UTXO::SetOutput(const OutputPtr &o) {
			_output = o;
		}

		uint32_t UTXO::GetConfirms(uint32_t lastBlockHeight) const {
			if (_blockHeight == TX_UNCONFIRMED)
				return 0;

			return lastBlockHeight >= _blockHeight ? lastBlockHeight - _blockHeight + 1 : 0;
		}

		bool UTXO::Equal(const InputPtr &input) const {
			return _hash == input->TxHash() && _n == input->Index();
		}

		bool UTXO::Equal(const uint256 &hash, uint16_t index) const {
			return _hash == hash && index == _n;
		}

	}
}

