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

#include "TableBase.h"

#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		TableBase::TableBase(Sqlite *sqlite) :
				_sqlite(sqlite),
				_txType(IMMEDIATE) {
		}

		TableBase::TableBase(SqliteTransactionType type, Sqlite *sqlite) :
				_sqlite(sqlite),
				_txType(type) {
		}

		void TableBase::InitializeTable() {

		}

		TableBase::~TableBase() {

		}

		void TableBase::flush() {
			_sqlite->flush();
		}

		bool TableBase::ContainTable(const std::string &tableName) const {
			std::string sql;
			int count = 0;

			sql = "select count(*)  from sqlite_master where type='table' and name = '" + tableName + "';";

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

		bool TableBase::DoTransaction(const boost::function<bool()> &fun) const {

			bool result;
			_sqlite->BeginTransaction(_txType);
			try {
				result = fun();
			} catch (const std::exception &e) {
				result = false;
				Log::error("Data base error: {}", e.what());
			} catch (...) {
				result = false;
				Log::error("Unknown data base error.");
			}
			_sqlite->EndTransaction();

			return result;
		}

		bool TableBase::DeleteAll(const std::string &tableName) {
			return DoTransaction([&tableName, this]() {
				std::string sql = "DELETE FROM " + tableName + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
		}

		void TableBase::InitializeTable(const std::string &constructScript) {
			_sqlite->BeginTransaction(_txType);
			_sqlite->exec(constructScript, nullptr, nullptr);
			_sqlite->EndTransaction();
		}
	}
}