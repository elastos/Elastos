// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRInt.h>
#include "TransactionCreationParams.h"

namespace Elastos {
	namespace SDK {

		TxParam::TxParam() :
				_toAddress(""),
				_amount(0),
				_assetId() {

		}

		TxParam::~TxParam() {

		}

		const std::string &TxParam::getToAddress() const {
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

		const UInt256 &TxParam::getAssetId() const {
			return _assetId;
		}

		void TxParam::setAssetId(const UInt256 &id) {
			_assetId = id;
		}

		const std::string &TxParam::getFromAddress() const {
			return _fromAddress;
		}

		void TxParam::setFromAddress(const std::string &address) {
			_fromAddress = address;
		}

		uint64_t TxParam::getFee() const {
			return _fee;
		}

		void TxParam::setFee(uint64_t fee) {
			_fee = fee;
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

		TxParam *TxParamFactory::createTxParam(const std::string &fromAddress, const std::string &toAddress,
													 double amount, double fee, const std::string &memo) {
			TxParam *result = nullptr;
			TxType type = Normal; //todo init type by parameters
			switch (type) {
				case Normal:
					result = new TxParam;
					break;
				case Deposit:
					DepositTxParam *depositTxParam = new DepositTxParam;
					result = depositTxParam;
					break;
				case Withdraw:
					WithdrawTxParam *withdrawTxParam = new WithdrawTxParam;
					result = withdrawTxParam;
					break;
				case ID:
					IdTxParam *idTxParam = new IdTxParam;
					result = idTxParam;
					break;
			}
			result->setFromAddress(fromAddress);
			result->setToAddress(toAddress);
			result->setAmount(amount);
			result->setFee(fee);
			return result;
		}
	}
}