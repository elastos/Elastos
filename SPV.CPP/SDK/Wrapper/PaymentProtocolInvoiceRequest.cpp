// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolInvoiceRequest.h"

namespace Elastos {
	namespace SDK {
		PaymentProtocolInvoiceRequest::PaymentProtocolInvoiceRequest(BRKey senderPublickKey, uint64_t amount,
																	 const std::string &pkiType,
																	 const CMBlock &pkiData,
																	 const std::string &memo,
																	 const std::string &notifyURL,
																	 const CMBlock &signature) {
			_protocolInvoiceRequest = BRPaymentProtocolInvoiceRequestNew(&senderPublickKey, amount, pkiType.c_str(),
																		 pkiData, pkiData.GetSize(), memo.c_str(),
																		 notifyURL.c_str(), signature,
																		 signature.GetSize());
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

		CMBlock PaymentProtocolInvoiceRequest::getPKIData() const {
			CMBlock ret(_protocolInvoiceRequest->pkiDataLen);
			memcpy(ret, _protocolInvoiceRequest->pkiData, _protocolInvoiceRequest->pkiDataLen);

			return ret;
		}

		std::string PaymentProtocolInvoiceRequest::getMemo() const {
			return _protocolInvoiceRequest->memo;
		}

		std::string PaymentProtocolInvoiceRequest::getNotifyURL() const {
			return _protocolInvoiceRequest->notifyUrl;
		}

		CMBlock PaymentProtocolInvoiceRequest::getSignature() const {
			CMBlock ret(_protocolInvoiceRequest->sigLen);
			memcpy(ret, _protocolInvoiceRequest->signature, _protocolInvoiceRequest->sigLen);

			return ret;
		}

		CMBlock PaymentProtocolInvoiceRequest::serialize() const {
			size_t dataLen = BRPaymentProtocolInvoiceRequestSerialize(_protocolInvoiceRequest, nullptr, 0);
			CMBlock data(dataLen);
			BRPaymentProtocolInvoiceRequestSerialize(_protocolInvoiceRequest, data, dataLen);

			return data;
		}

	}
}