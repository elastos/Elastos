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

#define TABLE_NAME     "UsedAddressTable"
#define ADDRESS        "Address"
#define TABLE_CREATION "create table if not exists " TABLE_NAME "(" ADDRESS " text not null);"
		AddressUsed::AddressUsed(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
		}

		AddressUsed::~AddressUsed() {
		}

		void AddressUsed::InitializeTable() {
			TableBase::InitializeTable(TABLE_CREATION);
		}

		bool AddressUsed::Puts(const std::vector<std::string> &addresses) {
			return DoTransaction([&addresses, this]() {
				for (const std::string &address : addresses) {
					if (!this->PutInternal(address))
						return false;
				}

				return true;
			});
		}

		std::vector<std::string> AddressUsed::Gets() const {
			std::vector<std::string> addresses;
			int r;
			std::string sql("SELECT " ADDRESS " FROM " TABLE_NAME ";");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			while (SQLITE_ROW == (r = _sqlite->Step(stmt))) {
				std::string address;
				address = _sqlite->ColumnText(stmt, 0);
				addresses.push_back(address);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return addresses;
		}

		bool AddressUsed::DeleteAll() {
			return TableBase::DeleteAll(TABLE_NAME);
		}

		bool AddressUsed::PutInternal(const std::string &address) {
			std::string sql("INSERT INTO " TABLE_NAME "(" ADDRESS ") VALUES (?);");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, address, nullptr)) {
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