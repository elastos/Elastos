/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Common/Log.h>
#include "TransactionPending.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionPending::TransactionPending(Sqlite *sqlite, SqliteTransactionType type) :
			TransactionNormal(sqlite, type) {
			_tableName = "transactionPending";
			_tableExist = TableExistInternal();
		}

		TransactionPending::~TransactionPending() {
		}

		bool TransactionPending::TableExist() const {
			return _tableExist;
		}

		bool TransactionPending::TableExistInternal() const {
			int count = 0;

			std::string sql = "select count(*) from sqlite_master where type='table' and name = '" + _tableName + "';";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Coinbase update finalize");
				return false;
			}

			return count > 0;
		}
	}
}