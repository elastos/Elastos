// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H__

#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolMessage :
			public Wrapper<BRPaymentProtocolMessage> {

		public:
			PaymentProtocolMessage(BRPaymentProtocolMessageType type, ByteData message, uint64_t statusCode,
								   std::string statusMsg, ByteData identifier);

			~PaymentProtocolMessage();

			virtual std::string toString() const;

			virtual BRPaymentProtocolMessage *getRaw() const;

			BRPaymentProtocolMessageType getMessageType() const;

			ByteData getMessage() const;

			uint64_t getStatusCode() const;

			std::string geStatusMessage() const;

			ByteData getIdentifier() const;

			ByteData serialize();

		private:
			BRPaymentProtocolMessage *_protocolMessage = nullptr;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H
