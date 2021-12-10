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
			_n(0) {
		}

		UTXO::UTXO(const UTXO &u) {
			this->operator=(u);
		}

		UTXO &UTXO::operator=(const UTXO &u) {
            this->_address = u._address;
            this->_amount = u._amount;
			this->_hash = u._hash;
            this->_n = u._n;
			return *this;
		}

		UTXO::UTXO(const uint256 &hash, uint16_t n, const Address &address, const BigInt &amount) :
			_hash(hash),
			_n(n),
			_address(address),
			_amount(amount) {
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

		const Address &UTXO::GetAddress() const {
			return _address;
		}

		void UTXO::SetAddress(const Address &address) {
			_address = address;
		}

        const BigInt &UTXO::GetAmount() const {
            return _amount;
		}

        void UTXO::SetAmount(const BigInt &amount) {
            _amount = amount;
		}

		bool UTXO::Equal(const InputPtr &input) const {
			return _hash == input->TxHash() && _n == input->Index();
		}

		bool UTXO::Equal(const uint256 &hash, uint16_t index) const {
			return _hash == hash && index == _n;
		}

	}
}

