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
#include <Common/ByteStream.h>
#include "TxTable.h"

namespace Elastos {
	namespace ElaWallet {

		void TxEntity::SetTxHash(const std::string &h) { _txHash = h; }
		const std::string &TxEntity::GetTxHash() const { return _txHash; }
		void TxEntity::SetHeight(uint32_t h) { _height = h; }
		uint32_t TxEntity::GetHeight() const { return _height; }
		void TxEntity::SetTimestamp(time_t t) { _timestamp = t; }
		time_t TxEntity::GetTimestamp() const { return _timestamp; }
		void TxEntity::SetType(uint8_t t) { _type = t; }
		uint8_t TxEntity::GetType() const { return _type; }
		void TxEntity::SetVersion(uint8_t v) { _version = v; }
		uint8_t TxEntity::GetVersion() const { return _version; }
		void TxEntity::SetLockTime(time_t t) { _lockTime = t; }
		time_t TxEntity::GetLockTime() const { return _lockTime; }
		void TxEntity::SetPayloadVersion(uint8_t v) { _payloadVersion = v; }
		uint8_t TxEntity::GetPayloadVersion() const { return _payloadVersion; }
		void TxEntity::SetPayload(const bytes_t &p) { _payload = p; }
		const bytes_t &TxEntity::GetPayload() const { return _payload; }
		void TxEntity::SetOutputs(const bytes_t &o) { _outputs = o; }
		const bytes_t &TxEntity::GetOutputs() const { return _outputs; }
		void TxEntity::SetAttributes(const bytes_t &a) { _attributes = a; }
		const bytes_t &TxEntity::GetAttributes() const { return _attributes; }
		void TxEntity::SetInputs(const bytes_t &in) { _inputs = in; }
		const bytes_t &TxEntity::GetInputs() const { return _inputs; }
		void TxEntity::SetPrograms(const bytes_t &p) { _programs = p; }
		const bytes_t &TxEntity::GetPrograms() const { return _programs; }

		///////////////////////////////////////////////////////////
		TxTable::TxTable(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction(_tableCreation);
			CreateIndexTxHash();
			CreateIndexType();
			CreateIndexTimestamp();
			CreateIndexHeight();
		}

		TxTable::~TxTable() {

		}

		const std::string &TxTable::GetTableName() const {
			return _tableName;
		}

		bool TxTable::ContainTx(const std::string &hash) const {
			bool contain = false;
			std::string sql;

			sql = "SELECT COUNT(*) FROM " + _tableName + " WHERE " + _txHash + " = ?;";

			if (!SqliteWrapper(sql, [this, &contain, &hash](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_ROW == _sqlite->Step(stmt)) {
					contain = 0 != _sqlite->ColumnInt(stmt, 0);
				}

				return true;
			})) {
				return false;
			}

			return contain;
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities, uint32_t height) const {
			std::string sql = "SELECT * FROM " + _tableName + " WHERE " + _height + " >= ?;";
			return GetFullTxCommon(sql, entities, [&height, this](sqlite3_stmt *stmt) {
				return _sqlite->BindInt(stmt, 1, height);
			});
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities, const std::vector<uint8_t> &types) const {
			if (types.empty())
				return true;

			std::string sql = "SELECT * FROM " + _tableName + " WHERE " + _type + " IN (";

			for (size_t i = 0; i < types.size(); ++i)
				sql += "?,";
			sql.back() = ')';
			sql += ';';

			return GetFullTxCommon(sql, entities, [&types, this](sqlite3_stmt *stmt) {
				for (int i = 0; i < types.size(); ++i) {
					if (!_sqlite->BindInt(stmt, i + 1, types[i]))
						return false;
				}
				return true;
			});
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities) const {
			std::string sql = "SELECT * FROM " + _tableName + ";";

			return GetFullTxCommon(sql, entities, [this](sqlite3_stmt *stmt) {
				return true;
			});
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities, const std::set<std::string> &hashes) const {
			auto it = hashes.cbegin();
			size_t varTotalCnt = hashes.size(), varCnt;
			for (size_t cnt = 0; cnt < varTotalCnt; cnt += varCnt) {
				varCnt = (varTotalCnt - cnt);
				if (varCnt > SQLITE_MAX_VARIABLE_NUMBER)
					varCnt = SQLITE_MAX_VARIABLE_NUMBER;

				std::string sql = "SELECT * FROM " + _tableName + " WHERE " + _txHash + " IN (";
				for (size_t i = 0; i < varCnt; ++i)
					sql += "?,";
				sql.back() = ')';
				sql += ";";

				if (!GetFullTxCommon(sql, entities, [&varCnt, &it, this](sqlite3_stmt *stmt) {
					for (int i = 0; i < varCnt; ++i, ++it) {
						if (!_sqlite->BindText(stmt, i + 1, *it, nullptr)) {
							return false;
						}
					}

					return true;
				})) {
					return false;
				}
			}

			return true;
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities, uint8_t type, bool invertMatch, size_t offset, size_t limit, bool desc) const {
			std::string sql, order, opt;

			if (desc) order = " DESC ";
			else order = " ASC ";

			if (invertMatch) opt = " != ";
			else opt = " = ";

			sql = "SELECT * FROM " + _tableName + " WHERE " + _type + opt + "? ORDER BY " + _height + order + "LIMIT ? OFFSET ?;";

			return GetFullTxCommon(sql, entities, [&type, &offset, &limit, this](sqlite3_stmt *stmt) {
				return _sqlite->BindInt(stmt, 1, type) && _sqlite->BindInt64(stmt, 2, limit) &&
					   _sqlite->BindInt64(stmt, 3, offset);

			});
		}

		bool TxTable::GetTx(std::vector<TxEntity> &entities, const std::string &utxoTable, const std::string &utxoColumn) const {
			std::string sql = "SELECT * FROM " + _tableName + " WHERE " + _txHash +
				" IN (SELECT " + utxoColumn + " FROM " + utxoTable + " GROUP BY " + utxoColumn + ");";

			return GetFullTxCommon(sql, entities, [this](sqlite3_stmt *stmt) {
				return true;
			});
		}

		size_t TxTable::GetTxCnt(uint8_t type, bool invertMatch) const {
			size_t count = 0;
			std::string opt;

			if (invertMatch) opt = " != ";
			else opt = " = ";

			std::string sql = "SELECT COUNT(" + _txHash + ") AS nums FROM " + _tableName + " WHERE " + _type  + opt + "?;";

			if (!SqliteWrapper(sql, [this, &type, &count](sqlite3_stmt *stmt) {
				if (!_sqlite->BindInt(stmt, 1, type)) {
					Log::error("bind args");
					return false;
				}
				if (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}
				return true;
			})) {
				count = 0;
			}

			return count;
		}

		size_t TxTable::GetAllTxCnt() const {
			size_t count = 0;

			std::string sql = "SELECT COUNT(" + _txHash + ") AS nums FROM " + _tableName + ";";

			if (!SqliteWrapper(sql, [this, &count](sqlite3_stmt *stmt) {
				if (SQLITE_ROW == _sqlite->Step(stmt))
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				return true;
			})) {
				count = 0;
			}

			return count;
		}

		time_t TxTable::GetEarliestTxTimestamp() const {
			time_t timestamp = 0;
			std::string sql;
			sql = "SELECT " + _timestamp + " FROM " + _tableName + " ORDER BY " + _timestamp + " ASC LIMIT 1;";

			if (!SqliteWrapper(sql, [this, &timestamp](sqlite3_stmt *stmt) {
				if (SQLITE_ROW == _sqlite->Step(stmt))
					timestamp = _sqlite->ColumnInt(stmt, 0);
				return true;
			})) {
				timestamp = 0;
			}

			return timestamp;
		}

		bool TxTable::PutTx(const std::vector<TxEntity> &entities) {
			std::vector<TxEntity> newEntities;
			for (const TxEntity &e : entities)
				if (!ContainTx(e._txHash))
					newEntities.push_back(e);

			return DoTransaction([&newEntities, this]() {
				for (const TxEntity &entity : newEntities) {
					std::string sql;

					sql = "INSERT INTO " + _tableName + "(" +
						  _txHash + "," + _height + "," + _timestamp + "," +
						  _type + "," + _version + "," + _lockTime + "," +
						  _payloadVersion + "," + _payload + "," + _outputs + "," +
						  _attributes + "," + _inputs + "," + _programs +
						  ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?);";

					if (!SqliteWrapper(sql, [this, &entity](sqlite3_stmt *stmt) {
						if (!_sqlite->BindText(stmt, 1, entity._txHash, nullptr) ||
							!_sqlite->BindInt(stmt, 2, entity._height) ||
							!_sqlite->BindInt64(stmt, 3, entity._timestamp) ||
							!_sqlite->BindInt(stmt, 4, entity._type) ||
							!_sqlite->BindInt(stmt, 5, entity._version) ||
							!_sqlite->BindInt(stmt, 6, entity._lockTime) ||
							!_sqlite->BindInt(stmt, 7, entity._payloadVersion) ||
							!_sqlite->BindBlob(stmt, 8, entity._payload, nullptr) ||
							!_sqlite->BindBlob(stmt, 9, entity._outputs, nullptr) ||
							!_sqlite->BindBlob(stmt, 10, entity._attributes, nullptr) ||
							!_sqlite->BindBlob(stmt, 11, entity._inputs, nullptr) ||
							!_sqlite->BindBlob(stmt, 12, entity._programs, nullptr)) {
							Log::error("bind args");
							return false;
						}
						if (SQLITE_DONE != _sqlite->Step(stmt)) {
							Log::error("step");
							return false;
						}
						return true;
					})) {
						return false;
					}
				}
				return true;
			});
		}

		bool TxTable::UpdateTx(const std::vector<std::string> &hashes, uint32_t height, time_t timestamp) {
			return DoTransaction([&hashes, &height, &timestamp, this]() {
				for (const std::string &hash: hashes) {
					std::string sql = "UPDATE " + _tableName + " SET " + _height + " = ?, " + _timestamp + " = ? " +
									  " WHERE " + _txHash + " = ?;";

					if (!SqliteWrapper(sql, [this, &hash, &height, &timestamp](sqlite3_stmt *stmt) {
						if (!_sqlite->BindInt(stmt, 1, height) ||
							!_sqlite->BindInt64(stmt, 2, timestamp) ||
							!_sqlite->BindText(stmt, 3, hash, nullptr)) {
							Log::error("bind args");
							return false;
						}

						if (SQLITE_DONE != _sqlite->Step(stmt)) {
							Log::error("step");
							return false;
						}

						return true;
					})) {
						return false;
					}
				}

				return true;
			});
		}

		bool TxTable::DeleteTx(const std::string &hash) {
			return DoTransaction([this, &hash]() {
				std::string sql = "DELETE FROM " + _tableName + " WHERE " + _txHash + " = ?;";
				return SqliteWrapper(sql, [this, &hash](sqlite3_stmt *stmt) {
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

		bool TxTable::DeleteAll() {
			return TableBase::DeleteAll(_tableName);
		}

		bool TxTable::CreateIndexTxHash() {
			std::string sql = "CREATE UNIQUE INDEX IF NOT EXISTS " + _indexTxHash + " ON " + _tableName + " (" + _txHash + ")";
			return ExecInTransaction(sql);
		}

		bool TxTable::CreateIndexType() {
			std::string sql = "CREATE INDEX IF NOT EXISTS " + _indexType + " ON " + _tableName + " (" + _type + ")";
			return ExecInTransaction(sql);
		}

		bool TxTable::CreateIndexTimestamp() {
			std::string sql = "CREATE INDEX IF NOT EXISTS " + _indexTimestamp + " ON " + _tableName + " (" + _timestamp + ")";
			return ExecInTransaction(sql);
		}

		bool TxTable::CreateIndexHeight() {
			std::string sql = "CREATE INDEX IF NOT EXISTS " + _indexHeight + " ON " + _tableName + " (" + _height+ ")";
			return ExecInTransaction(sql);
		}

		bool TxTable::GetFullTxCommon(const std::string &sql, std::vector<TxEntity> &entities,
									  const boost::function<bool(sqlite3_stmt *stmt)> &bindArgs) const {
			return SqliteWrapper(sql, [this, &entities, &bindArgs](sqlite3_stmt *stmt) {
				if (!bindArgs(stmt)) {
					Log::error("bind args");
					return false;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					TxEntity entity;
					const uint8_t *pblob;

					entity._txHash = _sqlite->ColumnText(stmt, 0);
					entity._height = _sqlite->ColumnInt(stmt, 1);
					entity._timestamp = _sqlite->ColumnInt64(stmt, 2);
					entity._type = _sqlite->ColumnInt(stmt, 3);
					entity._version = _sqlite->ColumnInt(stmt, 4);
					entity._lockTime = _sqlite->ColumnInt(stmt, 5);
					entity._payloadVersion = _sqlite->ColumnInt(stmt, 6);

					pblob = (const uint8_t *)_sqlite->ColumnBlob(stmt, 7);
					entity._payload.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 7));

					pblob = (const uint8_t *)_sqlite->ColumnBlob(stmt, 8);
					entity._outputs.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 8));

					pblob = (const uint8_t *)_sqlite->ColumnBlob(stmt, 9);
					entity._attributes.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 9));

					pblob = (const uint8_t *)_sqlite->ColumnBlob(stmt, 10);
					entity._inputs.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 10));

					pblob = (const uint8_t *)_sqlite->ColumnBlob(stmt, 11);
					entity._programs.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 11));

					entities.push_back(entity);
				}

				return true;
			});
		}

		/////////////////////////////////////////////////////////
		const std::string &TxOldEntity::GetTxHash() const {
			return _id;
		}

		const bytes_t &TxOldEntity::GetBuf() const {
			return _buf;
		}

		uint32_t TxOldEntity::GetBlockHeight() const {
			return _blockHeight;
		}

		time_t TxOldEntity::GetTimestamp() const {
			return _timestamp;
		}

		const std::string &TxOldEntity::GetISO() const {
			return _iso;
		}

		bool TxTable::GetAllOldTx(std::vector<TxOldEntity> &entities) const {
			std::string sql;
			if (ContainTable(_tableNameNormal)) {
				sql = "SELECT " + _oldTxHash + "," + _oldBuff + "," + _oldBlockHeight + "," +
								  _oldTimestamp + "," + _oldiso + " FROM " + _tableNameNormal + ";";
				if (!GetOldTxCommon(sql, entities))
					Log::error("get old normal tx fail");
			}

			if (ContainTable(_tableNameCoinbase)) {
				sql = "SELECT " + _oldTxHash + "," + _oldBuff + "," + _oldBlockHeight + "," +
					  _oldTimestamp + "," + _oldiso + " FROM " + _tableNameCoinbase + ";";
				if (!GetOldTxCommon(sql, entities))
					Log::error("get old coinbase tx fail");
			}

			if (ContainTable(_tableNamePending)) {
				sql = "SELECT " + _oldTxHash + "," + _oldBuff + "," + _oldBlockHeight + "," +
					  _oldTimestamp + "," + _oldiso + " FROM " + _tableNamePending + ";";
				if (!GetOldTxCommon(sql, entities))
					Log::error("get old pending tx fail");
			}

			return true;
		}

		bool TxTable::RemoveOldTxTableInner() {
			bool r = true;
			std::vector<std::string> tables = {
				_tableNamePending,
				_tableNameNormal,
				_tableNameCoinbase,
				_txHashProposalTable,
				_txHashDPoSTable,
				_txHashDIDTable,
				_txHashCRCTable
			};

			for (std::string &table : tables) {
				std::string sql = "DROP TABLE IF EXISTS " + table + ";";
				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					r = false;
					Log::error("exec sql: {}", sql);
				}
			}

			return r;
		}

		bool TxTable::GetOldTxCommon(const std::string &sql, std::vector<TxOldEntity> &entities) const {
			return SqliteWrapper(sql, [&entities, this](sqlite3_stmt *stmt) {
				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					const uint8_t *pblob;
					TxOldEntity entity;
					entity._id = _sqlite->ColumnText(stmt, 0);

					pblob = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					entity._buf.assign(pblob, pblob + _sqlite->ColumnBytes(stmt, 1));

					entity._blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					entity._timestamp = (time_t) _sqlite->ColumnInt(stmt, 3);
					entity._iso = _sqlite->ColumnText(stmt, 4);

					entities.push_back(entity);
				}

				return true;
			});
		}

	}
}