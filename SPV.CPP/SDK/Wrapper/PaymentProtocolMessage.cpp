// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolMessage.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolMessage::PaymentProtocolMessage(BRPaymentProtocolMessageType type, const ByteData &message,
													   uint64_t statusCode, const std::string &statusMsg,
													   const ByteData &identifier) {
			_protocolMessage = BRPaymentProtocolMessageNew(type, message.data, message.length, statusCode,
														   statusMsg.c_str(), identifier.data,
														   identifier.length);
		}

		std::string PaymentProtocolMessage::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolMessage *PaymentProtocolMessage::getRaw() const {
			return _protocolMessage;
		}

		PaymentProtocolMessage::~PaymentProtocolMessage() {
			if (nullptr != _protocolMessage) {
				BRPaymentProtocolMessageFree(_protocolMessage);
			}
		}

		BRPaymentProtocolMessageType PaymentProtocolMessage::getMessageType() const {
			return _protocolMessage->msgType;
		}

		ByteData PaymentProtocolMessage::getMessage() const {
			return ByteData(_protocolMessage->message, _protocolMessage->msgLen);
		}

		uint64_t PaymentProtocolMessage::getStatusCode() const {
			return _protocolMessage->statusCode;
		}

		std::string PaymentProtocolMessage::geStatusMessage() const {
			return _protocolMessage->statusMsg;
		}

		ByteData PaymentProtocolMessage::getIdentifier() const {
			return ByteData(_protocolMessage->identifier, _protocolMessage->identLen);
		}

		ByteData PaymentProtocolMessage::serialize() {
			size_t dataLen = BRPaymentProtocolMessageSerialize(_protocolMessage, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolMessageSerialize(_protocolMessage, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}