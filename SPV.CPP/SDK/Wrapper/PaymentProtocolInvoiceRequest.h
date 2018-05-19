// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H__


#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "Key.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolInvoiceRequest :
			public Wrapper<BRPaymentProtocolInvoiceRequest> {

		public:
			PaymentProtocolInvoiceRequest(BRKey senderPublickKey, uint64_t amount, const std::string &pkiType,
										  const CMBlock &pkiData, const std::string &memo,
										  const std::string &notifyURL,
										  const CMBlock &signature);

			~PaymentProtocolInvoiceRequest();

			virtual std::string toString() const;

			virtual BRPaymentProtocolInvoiceRequest *getRaw() const;

			BRKey getSenderPublicKey() const;

			uint64_t getAmount() const;

			std::string getPKIType() const;

			CMBlock getPKIData() const;

			std::string getMemo() const;

			std::string getNotifyURL() const;

			CMBlock getSignature() const;

			CMBlock serialize() const;

		private:
			BRPaymentProtocolInvoiceRequest *_protocolInvoiceRequest;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLINVOICEREQUEST_H
