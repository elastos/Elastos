// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLENCRYPTEDMESSAGE_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLENCRYPTEDMESSAGE_H__

#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolEncryptedMessage :
			public Wrapper<BRPaymentProtocolEncryptedMessage> {

		public:
			PaymentProtocolEncryptedMessage(ByteData data);

			~PaymentProtocolEncryptedMessage();

			virtual std::string toString() const;

			virtual BRPaymentProtocolEncryptedMessage *getRaw() const;

			ByteData getMessage() const;

			BRKey getReceiverPublicKey() const;

			BRKey getSenderPublicKey() const;

			uint64_t getNonce() const;

			ByteData getSignature() const;

			ByteData getIdentifier() const;

			uint64_t getStatusCode() const;

			std::string getStatusMessage() const;

			ByteData serialize() const;

		private:
			BRPaymentProtocolEncryptedMessage *_protocolEncryptedMessage = nullptr;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLENCRYPTEDMESSAGE_H__
