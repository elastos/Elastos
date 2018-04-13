// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H__


#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "Key.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolInvoiceRequest :
			public Wrapper<BRPaymentProtocolInvoiceRequest> {

		public:
			PaymentProtocolInvoiceRequest(BRKey senderPublickKey, uint64_t amount, std::string pkiType,
										  ByteData pkiData, std::string memo, std::string notifyURL,
										  ByteData signature);

			~PaymentProtocolInvoiceRequest();

			virtual std::string toString() const;

			virtual BRPaymentProtocolInvoiceRequest *getRaw() const;

			BRKey getSenderPublicKey() const;

			uint64_t getAmount() const;

			std::string getPKIType() const;

			ByteData getPKIData() const;

			std::string getMemo() const;

			std::string getNotifyURL() const;

			ByteData getSignature() const;

			ByteData serialize() const;

		private:
			BRPaymentProtocolInvoiceRequest *_protocolInvoiceRequest;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H
