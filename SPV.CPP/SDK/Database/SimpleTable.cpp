/*
 * Copyright (c) 2020 Elastos Foundation
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

#include "SimpleTable.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		SimpleTable::SimpleTable(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			_tableName = "";
			_columnName = "";
			_tableCreation = "";
		}

		SimpleTable::~SimpleTable() {
		}

		void SimpleTable::InitializeTable() {
			TableBase::InitializeTable(_tableCreation);
		}

		bool SimpleTable::Puts(const std::vector<std::string> &items, bool replace) {
			return DoTransaction([&items, &replace, this]() {
				if (replace) {
					std::string sql("DELETE FROM " + _tableName + ";");

					if (!_sqlite->exec(sql, nullptr, nullptr)) {
						Log::error("exec sql: {}" + sql);
						return false;
					}
				}

				for (const std::string &item : items) {
					if (!this->PutInternal(item))
						return false;
				}

				return true;
			});
		}

		std::vector<std::string> SimpleTable::Gets() const {
			std::vector<std::string> items;
			int r;
			std::string sql("SELECT " + _columnName + " FROM " + _tableName + ";");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			while (SQLITE_ROW == (r = _sqlite->Step(stmt))) {
				std::string item;
				item = _sqlite->ColumnText(stmt, 0);
				items.push_back(item);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return items;
		}

		bool SimpleTable::DeleteAll() {
			return TableBase::DeleteAll(_tableName);
		}

		bool SimpleTable::ContainTable() const {
			return _existTable;
		}

		const std::string &SimpleTable::GetTableName() const {
			return _tableName;
		}

		const std::string &SimpleTable::GetTxHashColumnName() const {
			return _columnName;
		}

		bool SimpleTable::PutInternal(const std::string &item) {
			std::string sql("INSERT OR REPLACE INTO " + _tableName + "(" + _columnName + ") VALUES (?);");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, item, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("utxo put finalize");
				return false;
			}

			return true;
		}

	}
}