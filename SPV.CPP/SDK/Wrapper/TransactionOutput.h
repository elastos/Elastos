// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include "BRTransaction.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class TransactionOutput :
			public Wrapper<BRTxOutput *> {

		public:
			TransactionOutput(uint64_t amount, const ByteData &script);

			~TransactionOutput();

			virtual std::string toString() const;

			virtual BRTxOutput *getRaw() const;

			std::string getAddress() const;

			void setAddress(std::string address);

			uint64_t getAmount() const;

			void setAmount(uint64_t amount);

			ByteData getScript() const;

		private:
			BRTxOutput *_output;
		};
	}
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
