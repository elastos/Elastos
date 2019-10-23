// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NotifyQueue.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		NotifyQueue::NotifyQueue(const boost::filesystem::path &path)
			: TableBase(new Sqlite(path)) {
			InitializeTable(NOTIFY_QUEUE_TABLE_CREATE);
		}

		NotifyQueue::~NotifyQueue() {}

		bool NotifyQueue::Upsert(const RecordPtr &record) {
			return DoTransaction([&record, this]() {
				std::string sql, tx_hash = record->tx_hash.GetHex();

				sql = "REPLACE INTO " + NOTIFY_QUEUE_TABLE + "(" +
					  NOTIFY_QUEUE_COLUMN_TX_HASH + "," +
					  NOTIFY_QUEUE_COLUMN_HEIGHT + "," +
					  NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME + ") VALUES(?,?,?);";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindText(stmt, 1, tx_hash, nullptr) ||
					!_sqlite->BindInt(stmt, 2, record->height) ||
					!_sqlite->BindInt64(stmt, 3, record->last_notify_time)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("finalize");
					return false;
				}

				return true;
			});
		}

		NotifyQueue::Records NotifyQueue::GetAllConfirmed(uint32_t current) {
			Records rows;
			DoTransaction([&rows, current, this]() {
				std::string sql;

				sql = "SELECT " +
					  NOTIFY_QUEUE_COLUMN_TX_HASH + "," +
					  NOTIFY_QUEUE_COLUMN_HEIGHT + "," +
					  NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME +
					  " FROM " + NOTIFY_QUEUE_TABLE +
					  " WHERE " + NOTIFY_QUEUE_COLUMN_HEIGHT + " != 0 AND " + NOTIFY_QUEUE_COLUMN_HEIGHT + " <= ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindInt(stmt, 1, current - 5)) {
					Log::error("bind args");
					return false;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					RecordPtr row(new Record(uint256(_sqlite->ColumnText(stmt, 0)),
											 (uint32_t) _sqlite->ColumnInt(stmt, 1),
											 (time_t) _sqlite->ColumnInt(stmt, 2)));
					rows.push_back(row);
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("finalize");
					return false;
				}

				return true;
			});
			return rows;
		}

		bool NotifyQueue::Delete(const uint256 &tx_hash) {
			return DoTransaction([&tx_hash, this]() {
				std::string sql, hash = tx_hash.GetHex();

				sql = "DELETE FROM " + NOTIFY_QUEUE_TABLE +
					  " WHERE " + NOTIFY_QUEUE_COLUMN_TX_HASH + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("finalize");
					return false;
				}

				return true;
			});
		}

	} // namespace ElaWallet
} // namespace Elastos
