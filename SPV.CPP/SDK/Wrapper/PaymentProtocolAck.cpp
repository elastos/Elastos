// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolAck.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolAck::PaymentProtocolAck(const ByteData &data) {
			_protocolACK = BRPaymentProtocolACKParse(data.data, data.length);
		}

		PaymentProtocolAck::~PaymentProtocolAck() {
			if (nullptr != _protocolACK) {
				BRPaymentProtocolACKFree(_protocolACK);
			}
		}

		std::string PaymentProtocolAck::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolACK *PaymentProtocolAck::getRaw() const {
			return _protocolACK;
		}

		std::string PaymentProtocolAck::geCustomerMemo() const {
			return _protocolACK->memo;
		}

		ByteData PaymentProtocolAck::getMerchantData() const {
			return ByteData(_protocolACK->payment->merchantData, _protocolACK->payment->merchDataLen);
		}

		SharedWrapperList<Transaction, BRTransaction *> PaymentProtocolAck::getTransactions() const {
			size_t transactionCount = _protocolACK->payment->txCount;

			SharedWrapperList<Transaction, BRTransaction *> results;
			for (int index = 0; index < transactionCount; index++) {
				results.push_back(TransactionPtr(new Transaction(_protocolACK->payment->transactions[index])));
			}

			return results;
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> PaymentProtocolAck::getRefundTo() const {
			SharedWrapperList<TransactionOutput, BRTxOutput *> results;
			for (size_t index = 0; index < _protocolACK->payment->refundToCount; index++) {
				BRTxOutput brOutput = _protocolACK->payment->refundTo[index];
				TransactionOutput *txOutput = new TransactionOutput(brOutput.amount,
																	ByteData(brOutput.script, brOutput.scriptLen));
				results.push_back(TransactionOutputPtr(txOutput));
			}
			return results;
		}

		std::string PaymentProtocolAck::getMerchantMemo() const {
			return _protocolACK->payment->memo;
		}

		ByteData PaymentProtocolAck::serialize() const {
			size_t dataLen = BRPaymentProtocolACKSerialize(_protocolACK, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolACKSerialize(_protocolACK, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}