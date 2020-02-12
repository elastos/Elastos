// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UTXOStore.h"

#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

#define TABLE_NAME     "UTXOTable"
#define TX_HASH        "txHash"
#define OUTPUT_INDEX   "outputIndex"
#define TABLE_CREATION "create table if not exists " TABLE_NAME "(" \
						TX_HASH " text not null," OUTPUT_INDEX " integer);"

		UTXOStore::UTXOStore() :
			TableBase(nullptr),
			_tableExist(false) {
		}

		UTXOStore::UTXOStore(Sqlite *sqlite) : TableBase(sqlite) {
			_tableExist = TableExistInternal();
			InitializeTable(TABLE_CREATION);
		}

		UTXOStore::~UTXOStore() {
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
			return DoTransaction([this]() {
				std::string sql("DELETE FROM " TABLE_NAME ";");

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
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
