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

#include "NotifyQueue.h"

#include <Common/ByteStream.h>
#include <Common/ErrorChecker.h>
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		NotifyQueue::NotifyQueue(const boost::filesystem::path &path)
			: TableBase(new Sqlite(path)) {
			ExecInTransaction(NOTIFY_QUEUE_TABLE_CREATE);
		}

		NotifyQueue::~NotifyQueue() {
		    delete(this->_sqlite);
		}

		bool NotifyQueue::Upsert(const RecordPtr &record) {
			return DoTransaction([&record, this]() {
				std::string sql, tx_hash = record->tx_hash.GetHex();

				sql = "REPLACE INTO " + NOTIFY_QUEUE_TABLE + "(" +
					  NOTIFY_QUEUE_COLUMN_TX_HASH + "," +
					  NOTIFY_QUEUE_COLUMN_HEIGHT + "," +
					  NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME + ") VALUES(?,?,?);";

				return SqliteWrapper(sql, [this, &record, &tx_hash](sqlite3_stmt *stmt) {
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

					return true;
				});
			});
		}

		NotifyQueue::Records NotifyQueue::GetAllConfirmed(uint32_t current) {
			Records rows;
			std::string sql;

			sql = "SELECT " +
				  NOTIFY_QUEUE_COLUMN_TX_HASH + "," +
				  NOTIFY_QUEUE_COLUMN_HEIGHT + "," +
				  NOTIFY_QUEUE_COLUMN_LAST_NOTIFY_TIME +
				  " FROM " + NOTIFY_QUEUE_TABLE +
				  " WHERE " + NOTIFY_QUEUE_COLUMN_HEIGHT + " != 0 AND " + NOTIFY_QUEUE_COLUMN_HEIGHT + " <= ?;";

			if (!SqliteWrapper(sql, [&rows, &current, this](sqlite3_stmt *stmt) {
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

				return true;
			})) {
				return {};
			}

			return rows;
		}

		bool NotifyQueue::Delete(const uint256 &tx_hash) {
			return DoTransaction([&tx_hash, this]() {
				std::string sql, hash = tx_hash.GetHex();

				sql = "DELETE FROM " + NOTIFY_QUEUE_TABLE +
					  " WHERE " + NOTIFY_QUEUE_COLUMN_TX_HASH + " = ?;";

				return SqliteWrapper(sql, [&hash, this](sqlite3_stmt *stmt) {
					if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
						Log::error("bind args");
						return false;
					}

					if (SQLITE_DONE != _sqlite->Step(stmt)) {
						Log::error("step");
						return false;
					}

					return true;
				});
			});
		}

	} // namespace ElaWallet
} // namespace Elastos
