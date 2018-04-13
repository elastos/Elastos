// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLACK_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLACK_H__

#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "ByteData.h"
#include "Transaction.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolAck :
			public Wrapper<BRPaymentProtocolACK> {

		public:
			PaymentProtocolAck(const ByteData &data);

			~PaymentProtocolAck();

			virtual std::string toString() const;

			virtual BRPaymentProtocolACK *getRaw() const;

			std::string geCustomerMemo() const;

			ByteData getMerchantData() const;

			SharedWrapperList<Transaction, BRTransaction *> getTransactions() const;

			SharedWrapperList<TransactionOutput, BRTxOutput *> getRefundTo() const;

			std::string getMerchantMemo() const;

			ByteData serialize() const;

		private:
			BRPaymentProtocolACK *_protocolACK;

		};
	}

}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLACK_H__
