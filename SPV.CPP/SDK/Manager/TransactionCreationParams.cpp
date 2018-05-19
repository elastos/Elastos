// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionCreationParams.h"

namespace Elastos {
	namespace SDK {

		TxParam::TxParam() :
				_toAddress(""),
				_amount(0) {

		}

		TxParam::~TxParam() {

		}

		std::string TxParam::getToAddress() const {
			return _toAddress;
		}

		void TxParam::setToAddress(const std::string &address) {
			_toAddress = address;
		}

		uint64_t TxParam::getAmount() const {
			return _amount;
		}

		void TxParam::setAmount(uint64_t amount) {
			_amount = amount;
		}

		std::string DepositTxParam::getSidechainAddress() const {
			return _sidechainAddress;
		}

		void DepositTxParam::setSidechainAddress(const std::string &address) {
			_sidechainAddress = address;
		}

		std::string WithdrawTxParam::getMainchainAddress() const {
			return _mainchainAddress;
		}

		void WithdrawTxParam::setMainchainAddress(const std::string &address) {
			_mainchainAddress = address;
		}

		std::string IdTxParam::getId() const {
			return _id;
		}

		void IdTxParam::setId(const std::string &id) {
			_id = id;
		}

		const CMBlock &IdTxParam::getData() const {
			return _data;
		}

		void IdTxParam::setData(const CMBlock &data) {
			_data = data;
		}
	}
}