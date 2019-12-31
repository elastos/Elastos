// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "DIDDataStore.h"

#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		DIDDataStore::DIDDataStore(Sqlite *sqlite) : TableBase(sqlite) {
			InitializeTable(DID_DATABASE_CREATE);
			InitializeTable("drop table if exists " + DID_OLD_TABLE_NAME + ";");
		}

		DIDDataStore::DIDDataStore(SqliteTransactionType type, Sqlite *sqlite) : TableBase(type, sqlite) {
			InitializeTable(DID_DATABASE_CREATE);
			InitializeTable("drop table if exists " + DID_OLD_TABLE_NAME + ";");
		}

		DIDDataStore::~DIDDataStore() {

		}

		bool DIDDataStore::PutDID(const std::string &iso, const DIDEntity &didEntity) {
			if (ContainTxHash(didEntity.TxHash)) {
				Log::error("should not put in existed did {}, txHash {}", didEntity.DID, didEntity.TxHash);
				return false;
			}

			return DoTransaction([&iso, &didEntity, this]() {
				return InsertDID(iso, didEntity);
			});
		}

		bool DIDDataStore::UpdateDID(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			return DoTransaction([ &txHashes, &blockHeight, &timestamp, this]() {
				std::string sql, hash;

				for (size_t i = 0; i < txHashes.size(); ++i) {
					hash = txHashes[i].GetHex();
					sql = "UPDATE " + DID_TABLE_NAME + " SET " +
					      BLOCK_HEIGHT + " = ?, " +
					      TIME_STAMP + " = ? " +
					      " WHERE " + TX_HASH + " = ?;";

					sqlite3_stmt *stmt;
					if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
						Log::error("prepare sql: {}", sql);
						return false;
					}

					if (!_sqlite->BindInt(stmt, 1, blockHeight) ||
					    !_sqlite->BindInt64(stmt, 2, timestamp) ||
					    !_sqlite->BindText(stmt, 3, hash, nullptr)) {
						Log::error("bind args");
					}

					if (!_sqlite->Step(stmt)) {
						Log::error("step");
					}

					if (!_sqlite->Finalize(stmt)) {
						Log::error("DID update finalize");
						return false;
					}
				}
				return true;
			});
		}

		bool DIDDataStore::DeleteDID(const std::string &did) {
			return DoTransaction([&did, this]() {
				std::string sql;

				sql = "DELETE FROM " + DID_TABLE_NAME + " WHERE " + DID_COLUMN_ID + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("Prepare sql {}", sql);
					return false;
				}

				if (!_sqlite->BindText(stmt, 1, did, nullptr)) {
					Log::error("bind text");
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("DID delete finalize");
					return false;
				}

				return true;
			});
		}

		bool DIDDataStore::DeleteDIDByTxHash(const std::string &txHash)  {
			return DoTransaction([&txHash, this]() {
				std::string sql;

				sql = "DELETE FROM " + DID_TABLE_NAME + " WHERE " + TX_HASH + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("Prepare sql {}", sql);
					return false;
				}

				if (!_sqlite->BindText(stmt, 1, txHash, nullptr)) {
					Log::error("bind text");
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("DID delete by hash finalize");
					return false;
				}

				return true;
			});
		}

		bool DIDDataStore::DeleteAllDID() {
			return DoTransaction([this]() {
				std::string sql;

				sql = "DELETE FROM " + DID_TABLE_NAME + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
		}

		std::vector<DIDEntity> DIDDataStore::GetAllDID() const {
			std::vector<DIDEntity> didEntitys;

			DIDEntity didEntity;
			std::string sql;

			sql = "SELECT " + DID_COLUMN_ID + ", " + DID_PAYLOAD_BUFF + ", " + DID_CREATE_TIME + ", " + BLOCK_HEIGHT
				  + ", " + TIME_STAMP + ", " + TX_HASH + " FROM " + DID_TABLE_NAME + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			while (SQLITE_ROW == _sqlite->Step(stmt)) {

				didEntity.DID = _sqlite->ColumnText(stmt, 0);

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
				didEntity.PayloadInfo.assign(pdata, pdata + len);

				didEntity.CreateTime = _sqlite->ColumnInt64(stmt, 2);
				didEntity.BlockHeight = _sqlite->ColumnInt(stmt, 3);
				didEntity.TimeStamp = _sqlite->ColumnInt64(stmt, 4);
				didEntity.TxHash = _sqlite->ColumnText(stmt, 5);

				didEntitys.push_back(didEntity);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID get all finalize");
				return {};
			}

			return didEntitys;
		}

		std::string DIDDataStore::GetDIDByTxHash(const std::string &txHash) const {
			std::string did = "";

			std::string sql;

			sql = "SELECT " + DID_COLUMN_ID + " FROM " + DID_TABLE_NAME + " WHERE " + TX_HASH + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return "";
			}

			if (!_sqlite->BindText(stmt, 1, txHash, nullptr)) {
				Log::error("bind text");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				did = _sqlite->ColumnText(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID get by hash finalize");
				return "";
			}

			return did;
		}

		bool DIDDataStore::GetDIDDetails(const std::string &did, DIDEntity &didEntity) const {
			return SelectDID(did, didEntity);
		}

		bool DIDDataStore::InsertDID(const std::string &iso, const DIDEntity &didEntity) {
			std::string sql;

			sql = "INSERT INTO " + DID_TABLE_NAME + "(" + DID_COLUMN_ID + "," +
					DID_PAYLOAD_BUFF + "," + DID_CREATE_TIME + "," + BLOCK_HEIGHT + "," +
					TIME_STAMP + "," + TX_HASH + "," + DID_RESERVE + ") VALUES (?, ?, ?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, didEntity.DID, nullptr) ||
			    !_sqlite->BindBlob(stmt, 2, didEntity.PayloadInfo, nullptr) ||
			    !_sqlite->BindInt64(stmt, 3, didEntity.CreateTime) ||
			    !_sqlite->BindInt(stmt, 4, didEntity.BlockHeight) ||
			    !_sqlite->BindInt64(stmt, 5, didEntity.TimeStamp) ||
			    !_sqlite->BindText(stmt, 6, didEntity.TxHash, nullptr) ||
			    !_sqlite->BindText(stmt, 7, "", nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID insert finalize");
				return false;
			}

			return true;
		}

		bool DIDDataStore::UpdateDID(const std::string &iso, const DIDEntity &didEntity) {
			std::string sql;

			sql = "UPDATE " + DID_TABLE_NAME + " SET "
			      + DID_PAYLOAD_BUFF + " = ?, "
			      + BLOCK_HEIGHT + " = ?, "
			      + TIME_STAMP + " = ?, "
			      + TX_HASH + " = ?, "
			      + DID_RESERVE + " = ?"
			      + " WHERE " + DID_COLUMN_ID + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindBlob(stmt, 1, didEntity.PayloadInfo, nullptr) ||
			    !_sqlite->BindInt(stmt, 2, didEntity.BlockHeight) ||
			    !_sqlite->BindInt64(stmt, 3, didEntity.TimeStamp) ||
			    !_sqlite->BindText(stmt, 4, didEntity.TxHash, nullptr) ||
			    !_sqlite->BindText(stmt, 5, "", nullptr) ||
			    !_sqlite->BindText(stmt, 6, didEntity.DID, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID update finalize");
				return false;
			}

			return true;
		}

		bool DIDDataStore::SelectDID(const std::string &did, DIDEntity &didEntity) const {
			bool found = false;
			std::string sql;

			sql = "SELECT " + DID_PAYLOAD_BUFF + ", " + DID_CREATE_TIME + "," + BLOCK_HEIGHT + ", " + TIME_STAMP + ", "
				+ TX_HASH + " FROM " + DID_TABLE_NAME + " WHERE " + DID_COLUMN_ID + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, did, nullptr)) {
				Log::error("bind text");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				found = true;

				didEntity.DID = did;

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 0);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 0);
				didEntity.PayloadInfo.assign(pdata, pdata + len);

				didEntity.CreateTime = _sqlite->ColumnInt64(stmt, 1);
				didEntity.BlockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
				didEntity.TimeStamp = (time_t) _sqlite->ColumnInt64(stmt, 3);
				didEntity.TxHash = _sqlite->ColumnText(stmt, 4);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID select finalize");
				return false;
			}

			return found;
		}

		bool DIDDataStore::ContainTxHash(const std::string &txHash) const {
			bool contain = false;

			std::string sql;

			sql = "SELECT " +
				  DID_COLUMN_ID + "," +
				  DID_PAYLOAD_BUFF + "," +
				  DID_CREATE_TIME + "," +
				  BLOCK_HEIGHT + "," +
				  TIME_STAMP + "," +
				  DID_RESERVE +
				  " FROM " + DID_TABLE_NAME +
				  " WHERE " + TX_HASH + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, txHash, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				contain = true;
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("DID contain finalize");
				return false;
			}

			return contain;
		}

		void DIDDataStore::flush() {
			_sqlite->flush();
		}
	}
}