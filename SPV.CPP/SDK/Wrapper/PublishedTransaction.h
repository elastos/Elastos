// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PUBLISHEDTRANSACTION_H__
#define __ELASTOS_SDK_PUBLISHEDTRANSACTION_H__

#include <boost/function.hpp>

#include "Transaction/Transaction.h"

namespace Elastos {
	namespace ElaWallet {

		class PublishedTransaction {
		public:
			PublishedTransaction();

			bool HasCallback() const;

		private:
			TransactionPtr _tx;
			boost::function<void (int)> _callback;
		};

	}
}

#endif //__ELASTOS_SDK_PUBLISHEDTRANSACTION_H__
