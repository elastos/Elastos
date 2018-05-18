// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PaymentProtocolEncryptedMessage.h"

namespace Elastos {
	namespace SDK {

		PaymentProtocolEncryptedMessage::PaymentProtocolEncryptedMessage(const CMBlock &data) {
			_protocolEncryptedMessage = BRPaymentProtocolEncryptedMessageParse(data, data.GetSize());
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

		CMBlock PaymentProtocolEncryptedMessage::getMessage() const {
			CMBlock ret(_protocolEncryptedMessage->msgLen);
			memcpy(ret, _protocolEncryptedMessage->message, _protocolEncryptedMessage->msgLen);

			return ret;
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

		CMBlock PaymentProtocolEncryptedMessage::getSignature() const {
			CMBlock ret(_protocolEncryptedMessage->msgLen);
			memcpy(ret, _protocolEncryptedMessage->message, _protocolEncryptedMessage->msgLen);

			return ret;
		}

		CMBlock PaymentProtocolEncryptedMessage::getIdentifier() const {
			CMBlock ret(_protocolEncryptedMessage->identLen);
			memcpy(ret, _protocolEncryptedMessage->identifier, _protocolEncryptedMessage->identLen);

			return ret;
		}

		uint64_t PaymentProtocolEncryptedMessage::getStatusCode() const {
			return _protocolEncryptedMessage->statusCode;
		}

		std::string PaymentProtocolEncryptedMessage::getStatusMessage() const {
			return _protocolEncryptedMessage->statusMsg;
		}

		CMBlock PaymentProtocolEncryptedMessage::serialize() const {
			size_t dataLen = BRPaymentProtocolEncryptedMessageSerialize(_protocolEncryptedMessage, nullptr, 0);
			CMBlock data(dataLen);
			BRPaymentProtocolEncryptedMessageSerialize(_protocolEncryptedMessage, data, dataLen);

			return data;
		}

	}
}
