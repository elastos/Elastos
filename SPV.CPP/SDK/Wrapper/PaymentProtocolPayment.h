// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYMENTPROTOCOLPAYMENT_H__
#define __ELASTOS_SDK_PAYMENTPROTOCOLPAYMENT_H__

#include "BRPaymentProtocol.h"

#include "Wrapper.h"
#include "c_util.h"
#include "Transaction.h"

namespace Elastos {
	namespace SDK {

		class PaymentProtocolPayment :
			public Wrapper<BRPaymentProtocolPayment> {

		public:
			PaymentProtocolPayment(const CMBlock &data);

			~PaymentProtocolPayment();

			virtual std::string toString() const;

			virtual BRPaymentProtocolPayment *getRaw() const;

			CMBlock getMerchantData() const;

			SharedWrapperList<Transaction, BRTransaction *> getTransactions() const;

			SharedWrapperList<TransactionOutput, BRTxOutput *> getRefundTo() const;

			std::string getMerchantMemo() const;

			CMBlock serialize() const;

		private:
			BRPaymentProtocolPayment *_protocolPayment = nullptr;
		};

	}
}

#endif //__ELASTOS_SDK_PAYMENTPROTOCOLPAYMENT_H__
