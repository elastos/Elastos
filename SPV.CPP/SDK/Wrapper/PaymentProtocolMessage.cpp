// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolMessage.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolMessage::PaymentProtocolMessage(BRPaymentProtocolMessageType type, const CMBlock &message,
													   uint64_t statusCode, const std::string &statusMsg,
													   const CMBlock &identifier) {
			_protocolMessage = BRPaymentProtocolMessageNew(type, message, message.GetSize(), statusCode,
														   statusMsg.c_str(), identifier, identifier.GetSize());
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

		CMBlock PaymentProtocolMessage::getMessage() const {
			CMBlock ret(_protocolMessage->msgLen);
			memcpy(ret, _protocolMessage->message, _protocolMessage->msgLen);

			return ret;
		}

		uint64_t PaymentProtocolMessage::getStatusCode() const {
			return _protocolMessage->statusCode;
		}

		std::string PaymentProtocolMessage::geStatusMessage() const {
			return _protocolMessage->statusMsg;
		}

		CMBlock PaymentProtocolMessage::getIdentifier() const {
			CMBlock ret(_protocolMessage->identLen);
			memcpy(ret, _protocolMessage->identifier, _protocolMessage->identLen);

			return ret;
		}

		CMBlock PaymentProtocolMessage::serialize() {
			size_t dataLen = BRPaymentProtocolMessageSerialize(_protocolMessage, nullptr, 0);
			CMBlock data(dataLen);
			BRPaymentProtocolMessageSerialize(_protocolMessage, data, dataLen);

			return data;
		}

	}
}