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

#include "AddressUsed.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		AddressUsed::AddressUsed(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction(_tableCreation);
		}

		AddressUsed::~AddressUsed() {
		}

		void AddressUsed::InitializeTable() {
			TableBase::ExecInTransaction(_tableCreation);
		}

		bool AddressUsed::Puts(const std::vector<std::string> &items, bool replace) {
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

		std::vector<std::string> AddressUsed::Gets() const {
			std::vector<std::string> items;
			std::string sql("SELECT " + _columnName + " FROM " + _tableName + ";");

			if (!SqliteWrapper(sql, [&items, this](sqlite3_stmt *stmt) {
				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					std::string item;
					item = _sqlite->ColumnText(stmt, 0);
					items.push_back(item);
				}

				return true;
			})) {
				return {};
			}

			return items;
		}

		bool AddressUsed::DeleteAll() {
			return TableBase::DeleteAll(_tableName);
		}

		bool AddressUsed::PutInternal(const std::string &item) {
			std::string sql("INSERT OR REPLACE INTO " + _tableName + "(" + _columnName + ") VALUES (?);");
			return SqliteWrapper(sql, [&item, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, item, nullptr)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				return true;
			});
		}

	}
}