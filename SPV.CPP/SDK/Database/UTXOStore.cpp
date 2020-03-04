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

		UTXOStore::UTXOStore(Sqlite *sqlite, SqliteTransactionType type) : TableBase(type, sqlite) {
			_tableName = "UTXOTable";
			_txHash = "txHash";
			_index = "outputIndex";
		}

		UTXOStore::~UTXOStore() {
		}

		void UTXOStore::InitializeTable() {
			_tableCreation = "create table if not exists " + _tableName + "(" +
						_txHash + " text not null," + _index + " integer);";
			TableBase::InitializeTable(_tableCreation);
		}

		bool UTXOStore::PutInternal(const UTXOEntity &entity) {
			std::string sql("INSERT INTO " + _tableName + "(" + _txHash + "," + _index + ") VALUES (?, ?);");

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
				for (const UTXOEntity &entity : entities) {
					if (!this->PutInternal(entity))
						return false;
				}

				return true;
			});
		}

		std::vector<UTXOEntity> UTXOStore::Gets() const {
			std::vector<UTXOEntity> utxos;
			int r;
			std::string sql("SELECT " + _txHash + "," + _index + " FROM " + _tableName + ";");

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

		bool UTXOStore::Update(const std::vector<UTXOEntity> &added, const std::vector<UTXOEntity> &deleted, bool replace) {
			return DoTransaction([this, &added, &deleted, &replace]() {
				if (replace) {
					std::string sql("DELETE FROM " + _tableName + ";");

					if (!_sqlite->exec(sql, nullptr, nullptr)) {
						Log::error("exec sql: {}" + sql);
						return false;
					}
				}

				for (const UTXOEntity &entity : deleted) {
					if (!this->DeleteInternal(entity))
						return false;
				}

				for (const UTXOEntity &entity : added) {
					if (!this->PutInternal(entity))
						return false;
				}
				return true;
			});
		}

		bool UTXOStore::DeleteAll() {
			return TableBase::DeleteAll(_tableName);
		}

		bool UTXOStore::DeleteInternal(const UTXOEntity &entity) {
			std::string sql("DELETE FROM " + _tableName + " WHERE " + _txHash + " = ? AND " + _index + " = ?;");

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

		const std::string &UTXOStore::GetTableName() const {
			return _tableName;
		}

		const std::string &UTXOStore::GetTxHashColumnName() const {
			return _txHash;
		}

		const std::string &UTXOStore::GetIndexColumnName() const {
			return _index;
		}

	}
}
