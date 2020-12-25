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

#include <Common/Log.h>
#include "DataMigrate.h"

namespace Elastos {
	namespace ElaWallet {

		DataMigrate::DataMigrate(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction(_tableCreation);
		}

		DataMigrate::~DataMigrate() {

		}

		void DataMigrate::InitializeTable() {
			TableBase::ExecInTransaction(_tableCreation);
		}

		bool DataMigrate::SetDataMigrateDoneInner(const std::string &type) {
			std::string sql;

			sql = "INSERT OR REPLACE INTO " + _tableName + "(" +
				  _type + "," + _done + ") VALUES (?,?);";

			return SqliteWrapper(sql, [&type, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, type, nullptr) ||
					!_sqlite->BindInt(stmt, 2, 1)) {
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

		bool DataMigrate::SetDataMigrateDone(const std::string &type) {
			return DoTransaction([&type, this]() {
				return this->SetDataMigrateDoneInner(type);
			});
		}

		bool DataMigrate::IsDataMigrateDone(const std::string &type) const {
			bool done = false;
			std::string sql;

			sql = "SELECT " + _done + " FROM " + _tableName + " WHERE " + _type + " = ?;";

			if (!SqliteWrapper(sql, [&type, &done, this](sqlite3_stmt *stmt) {

				if (!_sqlite->BindText(stmt, 1, type, nullptr)) {
					Log::error("bind args");
					return false;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					done = _sqlite->ColumnInt(stmt, 0) == 1;
				}

				return true;
			})) {
				return false;
			}

			return done;
		}

	}
}