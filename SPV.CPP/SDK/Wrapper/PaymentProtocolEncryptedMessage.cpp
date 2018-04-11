// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolEncryptedMessage.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolEncryptedMessage::PaymentProtocolEncryptedMessage(ByteData data) {
			_protocolEncryptedMessage = BRPaymentProtocolEncryptedMessageParse(data.data, data.length);
		}

		PaymentProtocolEncryptedMessage::~PaymentProtocolEncryptedMessage() {
			if (nullptr != _protocolEncryptedMessage) {
				BRPaymentProtocolEncryptedMessageFree(_protocolEncryptedMessage);
			}
		}

		std::string PaymentProtocolEncryptedMessage::toString() const {
			//TODO complete me
			return "";
		}

		BRPaymentProtocolEncryptedMessage *PaymentProtocolEncryptedMessage::getRaw() const {
			return _protocolEncryptedMessage;
		}

		ByteData PaymentProtocolEncryptedMessage::getMessage() const {
			return ByteData(_protocolEncryptedMessage->message, _protocolEncryptedMessage->msgLen);
		}

		BRKey PaymentProtocolEncryptedMessage::getReceiverPublicKey() const {
			return _protocolEncryptedMessage->receiverPubKey;
		}

		BRKey PaymentProtocolEncryptedMessage::getSenderPublicKey() const {
			return _protocolEncryptedMessage->senderPubKey;
		}

		uint64_t PaymentProtocolEncryptedMessage::getNonce() const {
			return _protocolEncryptedMessage->nonce;
		}

		ByteData PaymentProtocolEncryptedMessage::getSignature() const {
			return ByteData(_protocolEncryptedMessage->message, _protocolEncryptedMessage->msgLen);
		}

		ByteData PaymentProtocolEncryptedMessage::getIdentifier() const {
			return ByteData(_protocolEncryptedMessage->identifier, _protocolEncryptedMessage->identLen);
		}

		uint64_t PaymentProtocolEncryptedMessage::getStatusCode() const {
			return _protocolEncryptedMessage->statusCode;
		}

		std::string PaymentProtocolEncryptedMessage::getStatusMessage() const {
			return _protocolEncryptedMessage->statusMsg;
		}

		ByteData PaymentProtocolEncryptedMessage::serialize() const {
			size_t dataLen = BRPaymentProtocolEncryptedMessageSerialize(_protocolEncryptedMessage, nullptr, 0);
			uint8_t *data = new uint8_t[dataLen];
			BRPaymentProtocolEncryptedMessageSerialize(_protocolEncryptedMessage, data, dataLen);
			return ByteData(data, dataLen);
		}

	}
}
