// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/shared_ptr.hpp>

#include "BRPaymentProtocol.h"

#include "PaymentProtocolRequest.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolRequest::PaymentProtocolRequest(const ByteData &data) {
			_protocolRequest = BRPaymentProtocolRequestParse(data.data, data.length);
		}

		PaymentProtocolRequest::~PaymentProtocolRequest() {
			if (nullptr != _protocolRequest) {
				BRPaymentProtocolRequestFree(_protocolRequest);
			}
		}

		std::string PaymentProtocolRequest::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolRequest *PaymentProtocolRequest::getRaw() const {
			return _protocolRequest;
		}


		std::string PaymentProtocolRequest::getNetWork() const {
			return _protocolRequest->details->network;
		}

		SharedWrapperList<TransactionOutput, BRTxOutput *> PaymentProtocolRequest::getOutputs() const {
			SharedWrapperList<TransactionOutput, BRTxOutput *> results;
			for (size_t index = 0; index < _protocolRequest->details->outCount; index++) {
				BRTxOutput brOutput = _protocolRequest->details->outputs[index];
				TransactionOutput *txOutput = new TransactionOutput(brOutput.amount,
																	ByteData(brOutput.script, brOutput.scriptLen));
				results.push_back(TransactionOutputPtr(txOutput));
			}
			return results;
		}

		uint64_t PaymentProtocolRequest::getTime() const {
			return _protocolRequest->details->time;
		}

		uint64_t PaymentProtocolRequest::getExpires() const {
			return _protocolRequest->details->expires;
		}

		std::string PaymentProtocolRequest::getMemo() const {
			return _protocolRequest->details->memo;
		}

		std::string PaymentProtocolRequest::getPaymentURL() const {
			return _protocolRequest->details->paymentURL;
		}

		ByteData PaymentProtocolRequest::getMerchantData() const {
			return ByteData(_protocolRequest->details->merchantData,
							_protocolRequest->details->merchDataLen);
		}

		uint32_t PaymentProtocolRequest::getVersion() const {
			return _protocolRequest->version;
		}

		std::string PaymentProtocolRequest::getPKIType() const {
			return _protocolRequest->pkiType;
		}

		ByteData PaymentProtocolRequest::getPKIData() const {
			return ByteData(_protocolRequest->pkiData, _protocolRequest->pkiDataLen);
		}

		ByteData PaymentProtocolRequest::getSignature() const {
			return ByteData(_protocolRequest->signature, _protocolRequest->sigLen);
		}

		ByteData PaymentProtocolRequest::getDigest() const {
			size_t digestLen = BRPaymentProtocolRequestDigest(_protocolRequest, nullptr, 0);
			uint8_t *digestData = new uint8_t(digestLen);
			BRPaymentProtocolRequestDigest(_protocolRequest, digestData, digestLen);
			return ByteData(digestData, digestLen);
		}

		std::vector<ByteData> PaymentProtocolRequest::getCerts() const {
			std::vector<ByteData> certs;
			size_t numberOfCerts = 0;
			while (0 != BRPaymentProtocolRequestCert(_protocolRequest, nullptr, 0, numberOfCerts))
				numberOfCerts++;

			for (size_t index = 0; index < numberOfCerts; index++) {
				size_t certLen = BRPaymentProtocolRequestCert(_protocolRequest, nullptr, 0, index);
				uint8_t *certData = new uint8_t(certLen);
				BRPaymentProtocolRequestCert(_protocolRequest, certData, certLen, index);
				certs.push_back(ByteData(certData, certLen));
			}
			return certs;
		}

		ByteData PaymentProtocolRequest::serialize() const {
			size_t dataLen = BRPaymentProtocolRequestSerialize(_protocolRequest, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolRequestSerialize(_protocolRequest, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}