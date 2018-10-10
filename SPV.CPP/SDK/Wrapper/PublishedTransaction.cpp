// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PublishedTransaction.h"

namespace Elastos {
	namespace ElaWallet {

		PublishedTransaction::PublishedTransaction(const TransactionPtr &tx) :
				_tx(tx) {

		}

		PublishedTransaction::PublishedTransaction(const TransactionPtr &tx,
												   const boost::function<void(int)> &callback) :
				_tx(tx),
				_callback(callback) {
		}

		bool PublishedTransaction::HasCallback() const {
			return !_callback.empty();
		}

		const TransactionPtr &PublishedTransaction::GetTransaction() const {
			return _tx;
		}
	}
}

