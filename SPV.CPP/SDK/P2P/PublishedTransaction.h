// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PUBLISHEDTRANSACTION_H__
#define __ELASTOS_SDK_PUBLISHEDTRANSACTION_H__

#include <Plugin/Transaction/Transaction.h>

#include <boost/function.hpp>

namespace Elastos {
	namespace ElaWallet {

		typedef boost::function<void(const uint256 &, int, const std::string &)> PublishedTxCallback;

		class PublishedTransaction {
		public:
			PublishedTransaction();

			explicit PublishedTransaction(const TransactionPtr &tx);

			PublishedTransaction(const TransactionPtr &tx, const PublishedTxCallback &callback);

			void FireCallback(int code, const std::string &reason);

			bool HasCallback() const;

			void ResetCallback();

			const PublishedTxCallback &GetCallback() const;

			const TransactionPtr &GetTransaction() const;

		private:
			TransactionPtr _tx;
			PublishedTxCallback _callback;
		};

	}
}

#endif //__ELASTOS_SDK_PUBLISHEDTRANSACTION_H__
