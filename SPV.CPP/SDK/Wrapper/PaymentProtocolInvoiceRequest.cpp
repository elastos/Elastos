// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolInvoiceRequest.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolInvoiceRequest::PaymentProtocolInvoiceRequest(BRKey senderPublickKey, uint64_t amount,
																	 const std::string &pkiType,
																	 const ByteData &pkiData,
																	 const std::string &memo,
																	 const std::string &notifyURL,
																	 const ByteData &signature) {
			_protocolInvoiceRequest = BRPaymentProtocolInvoiceRequestNew(&senderPublickKey, amount,
																		 pkiType.c_str(), pkiData.data,
																		 pkiData.length, memo.c_str(),
																		 notifyURL.c_str(), signature.data,
																		 signature.length);
		}

		PaymentProtocolInvoiceRequest::~PaymentProtocolInvoiceRequest() {
			if (nullptr != _protocolInvoiceRequest) {
				BRPaymentProtocolInvoiceRequestFree(_protocolInvoiceRequest);
			}
		}

		std::string PaymentProtocolInvoiceRequest::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolInvoiceRequest *PaymentProtocolInvoiceRequest::getRaw() const {
			return _protocolInvoiceRequest;
		}

		BRKey PaymentProtocolInvoiceRequest::getSenderPublicKey() const {
			return _protocolInvoiceRequest->senderPubKey;
		}

		uint64_t PaymentProtocolInvoiceRequest::getAmount() const {
			return _protocolInvoiceRequest->amount;
		}

		std::string PaymentProtocolInvoiceRequest::getPKIType() const {
			return _protocolInvoiceRequest->pkiType;
		}

		ByteData PaymentProtocolInvoiceRequest::getPKIData() const {
			return ByteData(_protocolInvoiceRequest->pkiData, _protocolInvoiceRequest->pkiDataLen);
		}

		std::string PaymentProtocolInvoiceRequest::getMemo() const {
			return _protocolInvoiceRequest->memo;
		}

		std::string PaymentProtocolInvoiceRequest::getNotifyURL() const {
			return _protocolInvoiceRequest->notifyUrl;
		}

		ByteData PaymentProtocolInvoiceRequest::getSignature() const {
			return ByteData(_protocolInvoiceRequest->signature, _protocolInvoiceRequest->sigLen);
		}

		ByteData PaymentProtocolInvoiceRequest::serialize() const {
			size_t dataLen = BRPaymentProtocolInvoiceRequestSerialize(_protocolInvoiceRequest, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolInvoiceRequestSerialize(_protocolInvoiceRequest, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}