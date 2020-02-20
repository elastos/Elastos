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

#include "UTXOStore.h"

#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

#define TABLE_NAME     "UTXOTable"
#define TX_HASH        "txHash"
#define OUTPUT_INDEX   "outputIndex"
#define TABLE_CREATION "create table if not exists " TABLE_NAME "(" \
						TX_HASH " text not null," OUTPUT_INDEX " integer);"

		UTXOStore::UTXOStore(Sqlite *sqlite, SqliteTransactionType type) : TableBase(type, sqlite) {
			_tableExist = TableExistInternal();
		}

		UTXOStore::~UTXOStore() {
		}

		void UTXOStore::InitializeTable() {
			TableBase::InitializeTable(TABLE_CREATION);
		}

		bool UTXOStore::PutInternal(const UTXOEntity &entity) {
			std::string sql("INSERT INTO " TABLE_NAME "(" TX_HASH "," OUTPUT_INDEX ") VALUES (?, ?);");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, entity._hash, nullptr) ||
				!_sqlite->BindInt(stmt, 2, entity._n)) {
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

		bool UTXOStore::Puts(const std::vector<UTXOEntity> &entities) {
			if (entities.empty())
				return true;

			return DoTransaction([&entities, this]() {
				for (size_t i = 0; i < entities.size(); ++i) {
					if (!this->PutInternal(entities[i]))
						return false;
				}

				return true;
			});
		}

		std::vector<UTXOEntity> UTXOStore::Gets() const {
			std::vector<UTXOEntity> utxos;
			int r;
			std::string sql("SELECT " TX_HASH "," OUTPUT_INDEX " FROM " TABLE_NAME ";");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			while (SQLITE_ROW == (r = _sqlite->Step(stmt))) {
				UTXOEntity utxo;
				utxo._hash = _sqlite->ColumnText(stmt, 0);
				utxo._n = _sqlite->ColumnInt(stmt, 1);

				utxos.push_back(utxo);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return utxos;
		}

		bool UTXOStore::DeleteAll() {
			return TableBase::DeleteAll(TABLE_NAME);
		}

		bool UTXOStore::DeleteInternal(const UTXOEntity &entity) {
			std::string sql("DELETE FROM " TABLE_NAME " WHERE " TX_HASH " = ? AND " OUTPUT_INDEX " = ?;");

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, entity._hash, nullptr) ||
				!_sqlite->BindInt(stmt, 2, entity._n)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("stmp");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("utxo delete finalize");
				return false;
			}

			return true;
		}

		bool UTXOStore::Delete(const std::vector<UTXOEntity> &entities) {
			return DoTransaction([&entities, this]() {
				for (const UTXOEntity &entity : entities) {
					if (!this->DeleteInternal(entity))
						return false;
				}

				return true;
			});
		}

		bool UTXOStore::TableExist() const {
			return _tableExist;
		}

		bool UTXOStore::TableExistInternal() const {
			int count = 0;

			std::string sql("select count(*) from sqlite_master where type='table' and name = '" TABLE_NAME "';");

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
