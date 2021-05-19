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
#include "Settings.h"

namespace Elastos {
	namespace ElaWallet {

		Settings::Settings(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction(_tableCreation);
		}

		Settings::~Settings() {

		}

		bool Settings::PutSettingInner(const std::string &name, int value) {
			std::string sql;

			sql = "INSERT OR REPLACE INTO " + _tableName + "(" +
				  _name + "," + _value + ") VALUES (?,?);";

			return SqliteWrapper(sql, [&name, &value, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, name, nullptr) ||
					!_sqlite->BindInt(stmt, 2, value)) {
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

		bool Settings::PutSetting(const std::string &name, int value) {
			return DoTransaction([&name, &value, this]() {
				return this->PutSettingInner(name, value);
			});
		}

		int Settings::GetSetting(const std::string &name) const {
			int value = 0;
			std::string sql;

			sql = "SELECT " + _value + " FROM " + _tableName + " WHERE " + _name + " = ?;";

			if (!SqliteWrapper(sql, [&name, &value, this](sqlite3_stmt *stmt) {

				if (!_sqlite->BindText(stmt, 1, name, nullptr)) {
					Log::error("bind args");
					return false;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					value = _sqlite->ColumnInt(stmt, 0);
				}

				return true;
			})) {
				return value;
			}

			return value;
		}

	}
}