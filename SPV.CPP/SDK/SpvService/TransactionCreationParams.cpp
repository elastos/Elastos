// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRInt.h>
#include <SDK/ELACoreExt/Payload/Asset.h>
#include "Key.h"
#include "TransactionCreationParams.h"

namespace Elastos {
	namespace ElaWallet {

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

		const std::string &TxParam::getRemark() const {
			return _remark;
		}

		void TxParam::setRemark(const std::string &remark) {
			_remark = remark;
		}

		const std::string &TxParam::getMemo() const {
			return _memo;
		}

		void TxParam::setMemo(const std::string &memo) {
			_memo = memo;
		}

		std::string DepositTxParam::getSidechainAddress() const {
			return _sidechainAddress;
		}

		void DepositTxParam::setSidechainAddress(const std::string &address) {
			_sidechainAddress = address;
		}

		const std::vector<std::string> &DepositTxParam::getCrossChainAddress() const {
			return _crossChainAddress;
		}

		const std::vector<uint64_t> &DepositTxParam::getCrossChainOutputIndexs() const {
			return _outputIndex;
		}

		const std::vector<uint64_t> &DepositTxParam::getCrosschainAmouts() const {
			return _crossChainAmount;
		}

		void DepositTxParam::setSidechainDatas(const std::vector<std::string> &crossChainAddress,
											   const std::vector<uint64_t> &outputIndex,
											   const std::vector<uint64_t> &crossChainAmount) {
			_crossChainAddress = crossChainAddress;
			_outputIndex = outputIndex;
			_crossChainAmount = crossChainAmount;
		}

		std::string WithdrawTxParam::getMainchainAddress() const {
			return _mainchainAddress;
		}

		void WithdrawTxParam::setMainchainAddress(const std::string &address) {
			_mainchainAddress = address;
		}

		const std::vector<std::string> &WithdrawTxParam::getCrossChainAddress() const {
			return _crossChainAddress;
		}

		const std::vector<uint64_t> &WithdrawTxParam::getCrossChainOutputIndexs() const {
			return _outputIndex;
		}

		const std::vector<uint64_t> &WithdrawTxParam::getCrosschainAmouts() const {
			return _crossChainAmount;
		}

		void WithdrawTxParam::setMainchainDatas(const std::vector<std::string> crossChainAddress,
												const std::vector<uint64_t> outputIndex,
												const std::vector<uint64_t> crossChainAmount) {
			_crossChainAddress = crossChainAddress;
			_outputIndex = outputIndex;
			_crossChainAmount = crossChainAmount;
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

		TxParam *
		TxParamFactory::createTxParam(SubWalletType type, const std::string &fromAddress, const std::string &toAddress,
									  uint64_t amount, uint64_t fee, const std::string &memo,
									  const std::string &remark) {
			TxParam *result = nullptr;
			switch (type) {
				case Normal:
					result = new TxParam;
					break;
				case Mainchain:
					result = new DepositTxParam;
					break;
				case Sidechain:
					result = new WithdrawTxParam;
					break;
				case Idchain:
					result = new IdTxParam;
					break;
			}
			result->setFromAddress(fromAddress);
			result->setToAddress(toAddress);
			result->setAmount(amount);
			result->setFee(fee);
			result->setAssetId(Asset::GetELAAssetID());
			result->setRemark(remark);
			result->setMemo(memo);
			return result;
		}

		const PayloadRegisterProducer &RegisterProducerTxParam::GetPayload() const {
			return _payload;
		}

		void RegisterProducerTxParam::SetPayload(const PayloadRegisterProducer &payload) {
			_payload = payload;
		}

		const PayloadCancelProducer &CancelProducerTxParam::GetPayload() const {
			return _payload;
		}

		void CancelProducerTxParam::SetPayload(const PayloadCancelProducer &payload) {
			_payload = payload;
		}

		const PayloadVoteProducer &VoteProducerTxParam::GetPayload() const {
			return _payload;
		}

		void VoteProducerTxParam::SetPayload(const PayloadVoteProducer &payload) {
			_payload = payload;
		}
	}
}