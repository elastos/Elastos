// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TableBase.h"
#include "Log.h"

namespace Elastos {
	namespace ElaWallet {

#define SQLITE_MUTEX_LOCK_ON

		TableBase::TableBase(Sqlite *sqlite) :
				_sqlite(sqlite),
				_txType(EXCLUSIVE) {
		}

		TableBase::TableBase(SqliteTransactionType type, Sqlite *sqlite) :
				_sqlite(sqlite),
				_txType(type) {
		}

		TableBase::~TableBase() {

		}

		bool TableBase::doTransaction(const boost::function<void()> &fun) const {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			bool result = true;
			_sqlite->beginTransaction(_txType);
			try {
				fun();
			}
			catch (std::exception ex) {
				result = false;
				Log::getLogger()->error("Data base error: ", ex.what());
			}
			catch (...) {
				result = false;
				Log::error("Unknown data base error.");
			}
			_sqlite->endTransaction();

			return result;
		}

		void TableBase::initializeTable(const std::string &constructScript) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			_sqlite->beginTransaction(_txType);
			_sqlite->exec(constructScript, nullptr, nullptr);
			_sqlite->endTransaction();
		}
	}
}