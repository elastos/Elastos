// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TABLEBASE_H__
#define __ELASTOS_SDK_TABLEBASE_H__

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		class TableBase {
		public:
			TableBase(Sqlite *sqlite);

			TableBase(SqliteTransactionType type, Sqlite *sqlite);

			virtual ~TableBase();

		protected:
			void initializeTable(const std::string &constructScript);

			bool doTransaction(const boost::function<void()> &fun);

		protected:
			Sqlite *_sqlite;
			SqliteTransactionType _txType;
			mutable boost::mutex _lockMutex;
		};

	}
}

#endif //__ELASTOS_SDK_TABLEBASE_H__
