// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLREQUEST_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLREQUEST_H__

#include <vector>

#include "Wrapper.h"
#include "ByteData.h"
#include "TransactionOutput.h"
#include "SharedWrapperList.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolRequest :
			public Wrapper<BRPaymentProtocolRequest *> {

		public:
			PaymentProtocolRequest(ByteData data);

			~PaymentProtocolRequest();

			virtual std::string toString() const;

			virtual BRPaymentProtocolRequest *getRaw() const;

			std::string getNetWork() const;

			SharedWrapperList<TransactionOutput, BRTxOutput *> getOutputs() const;

			uint64_t getTime() const;

			uint64_t getExpires() const;

			std::string getMemo() const;

			std::string getPaymentURL() const;

			ByteData getMerchantData() const;

			uint32_t getVersion() const;

			std::string getPKIType() const;

			ByteData getPKIData() const;

			ByteData getSignature() const;

			ByteData getDigest() const;

			std::vector<ByteData> getCerts() const;

			ByteData serialize() const;

		private:
			BRPaymentProtocolRequest *_protocolRequest = nullptr;

		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLREQUEST_H__
