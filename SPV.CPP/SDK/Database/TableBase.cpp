// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TableBase.h"

#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

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

		bool TableBase::DoTransaction(const boost::function<void()> &fun) const {
			bool result = true;
			_sqlite->BeginTransaction(_txType);
			try {
				fun();
			}
			catch (std::exception ex) {
				result = false;
				Log::error("Data base error: ", ex.what());
			}
			catch (...) {
				result = false;
				Log::error("Unknown data base error.");
			}
			_sqlite->EndTransaction();

			return result;
		}

		void TableBase::InitializeTable(const std::string &constructScript) {
			_sqlite->BeginTransaction(_txType);
			_sqlite->exec(constructScript, nullptr, nullptr);
			_sqlite->EndTransaction();
		}
	}
}