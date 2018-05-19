// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H__

#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolMessage :
			public Wrapper<BRPaymentProtocolMessage> {

		public:
			PaymentProtocolMessage(BRPaymentProtocolMessageType type, const CMBlock &message, uint64_t statusCode,
								   const std::string &statusMsg, const CMBlock &identifier);

			~PaymentProtocolMessage();

			virtual std::string toString() const;

			virtual BRPaymentProtocolMessage *getRaw() const;

			BRPaymentProtocolMessageType getMessageType() const;

			CMBlock getMessage() const;

			uint64_t getStatusCode() const;

			std::string geStatusMessage() const;

			CMBlock getIdentifier() const;

			CMBlock serialize();

		private:
			BRPaymentProtocolMessage *_protocolMessage = nullptr;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLMESSAGE_H
