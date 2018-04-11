// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRPaymentProtocol.h>
#include "PaymentProtocolPayment.h"

namespace Elastos {
	namespace SDK {


		PaymentProtocolPayment::PaymentProtocolPayment(ByteData data) {
			_protocolPayment = BRPaymentProtocolPaymentParse(data.data, data.length);
		}

		PaymentProtocolPayment::~PaymentProtocolPayment() {
			if (nullptr != _protocolPayment) {
				BRPaymentProtocolPaymentFree(_protocolPayment);
			}
		}

		std::string PaymentProtocolPayment::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolPayment *PaymentProtocolPayment::getRaw() const {
			return _protocolPayment;
		}

		ByteData PaymentProtocolPayment::getMerchantData() const {
			return ByteData(_protocolPayment->merchantData, _protocolPayment->merchDataLen);
		}

		SharedWrapperList<Transaction, BRTransaction *> PaymentProtocolPayment::getTransactions() const {
			size_t transactionCount = _protocolPayment->txCount;

			SharedWrapperList<Transaction, BRTransaction *> results(transactionCount);
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction(_protocolPayment->transactions[index])));
			}

			return results;
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> PaymentProtocolPayment::getRefundTo() const {
			SharedWrapperList<TransactionOutput, BRTxOutput *> results(_protocolPayment->refundToCount);
			for (size_t index = 0; index < _protocolPayment->refundToCount; index++) {
				BRTxOutput brOutput = _protocolPayment->refundTo[index];
				TransactionOutput *txOutput = new TransactionOutput(brOutput.amount,
																	ByteData(brOutput.script, brOutput.scriptLen));
				results.push_back(TransactionOutputPtr(txOutput));
			}
			return results;
		}

		std::string PaymentProtocolPayment::getMerchantMemo() const {
			return _protocolPayment->memo;
		}

		ByteData PaymentProtocolPayment::serialize() const {
			size_t dataLen = BRPaymentProtocolPaymentSerialize(_protocolPayment, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolPaymentSerialize(_protocolPayment, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}