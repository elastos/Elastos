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

#include "TransactionNormal.h"

#include <Common/Log.h>
#include <Common/uint256.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Registry.h>

#include <string>

namespace Elastos {
	namespace ElaWallet {

		TransactionNormal::TransactionNormal(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			_tableName = "transactionTable";
			_txHash = "_id";
			_buff = "transactionBuff";
			_blockHeight = "transactionBlockHeight";
			_timestamp = "transactionTimeStamp";
			_iso = "transactionISO";
			_remark = "transactionRemark";
			_assetID = "assetID";
		}

		TransactionNormal::~TransactionNormal() {
		}

		void TransactionNormal::InitializeTable() {
			_tableCreation = "create table if not exists " +
							 _tableName + "(" +
							 _txHash + " text not null, " +
							 _buff + " blob, " +
							 _blockHeight + " integer, " +
							 _timestamp + " integer, " +
							 _remark + " text DEFAULT '', " +
							 _assetID + " text not null, " +
							 _iso + " text DEFAULT 'ELA');";
			TableBase::InitializeTable(_tableCreation);
		}

		bool TransactionNormal::_Put(const TransactionPtr &tx) {
			std::string sql, txHash;

			sql = "INSERT INTO " + _tableName + "(" +
				  _txHash + "," +
				  _buff + "," +
				  _blockHeight + "," +
				  _timestamp + "," +
				  _remark + "," +
				  _assetID + "," +
				  _iso + ") VALUES (?, ?, ?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			ByteStream stream;
			tx->Serialize(stream, true);

			txHash = tx->GetHash().GetHex();
			if (!_sqlite->BindText(stmt, 1, txHash, nullptr) ||
				!_sqlite->BindBlob(stmt, 2, stream.GetBytes(), nullptr) ||
				!_sqlite->BindInt(stmt, 3, tx->GetBlockHeight()) ||
				!_sqlite->BindInt64(stmt, 4, tx->GetTimestamp()) ||
				!_sqlite->BindText(stmt, 5, "", nullptr) ||
				!_sqlite->BindText(stmt, 6, "", nullptr) ||
				!_sqlite->BindText(stmt, 7, ISO, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx put finalize");
				return false;
			}

			return true;
		}

		bool TransactionNormal::Put(const TransactionPtr &tx) {
			if (ContainHash(tx->GetHash())) {
				Log::error("should not put in existed tx {}", tx->GetHash().GetHex());
				return false;
			}

			return DoTransaction([&tx, this]() { return this->_Put(tx); });
		}

		bool TransactionNormal::Puts(const std::vector<TransactionPtr> &txns) {
			return DoTransaction([&txns, this]() {
				return this->_Puts(txns, false);
			});
		}

		bool TransactionNormal::DeleteAll() {
			return TableBase::DeleteAll(_tableName);
		}

		size_t TransactionNormal::GetAllCount() const {
			size_t count = 0;

			std::string sql;

			sql = "SELECT COUNT(" + _txHash + ") AS nums FROM " + _tableName + ";";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return 0;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all count finalize");
				return 0;
			}

			return count;
		}

		time_t TransactionNormal::GetEarliestTxnTimestamp() const {
			time_t timestamp = 0;
			std::string sql;
			sql = "SELECT " + _timestamp + " FROM " + _tableName + " ORDER BY " + _timestamp + " ASC LIMIT 1;";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return timestamp;
			}

			while (SQLITE_ROW == _sqlite->Step(stmt)) {
				timestamp = _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return timestamp;
			}

			return timestamp;
		}

		std::vector<TransactionPtr> TransactionNormal::GetUniqueTxns(const std::string &chainID,
																	 const std::set<std::string> &uniqueHash) const {
			std::vector<TransactionPtr> txns;
			std::string sql;

			if (uniqueHash.empty())
				return txns;

			std::set<std::string>::iterator it = uniqueHash.cbegin();
			size_t cnt, maxCnt = uniqueHash.size(), markCnt;
			std::string mark;

			for (cnt = 0; cnt < maxCnt; ) {
				sql = "SELECT " + _txHash + "," + _buff + "," + _blockHeight + "," + _timestamp + "," + _iso +
					  " FROM " + _tableName + " WHERE " + _txHash + " IN (";

				markCnt = (maxCnt - cnt) < SQLITE_MAX_VARIABLE_NUMBER ? (maxCnt - cnt) : SQLITE_MAX_VARIABLE_NUMBER;

				for (size_t i = 0; i < markCnt; ++i)
					sql += "?,";
				sql.back() = ')';
				sql += ";";

				sqlite3_stmt *stmt = NULL;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return txns;
				}

				for (size_t i = 0; i < markCnt; ++i, ++it) {
					if (!_sqlite->BindText(stmt, (int)(i + 1), *it, nullptr)) {
						Log::error("bind args");
						break;
					}
				}

				GetSelectedTxns(txns, chainID, stmt);

				if (!_sqlite->Finalize(stmt)) {
					Log::error("Tx get all finalize");
					return {};
				}

				cnt += markCnt;
			}

			return txns;
		}

		TransactionPtr TransactionNormal::Get(const uint256 &hash, const std::string &chainID) const {
			return SelectByHash(hash, chainID);
		}

		std::vector<TransactionPtr> TransactionNormal::GetAfter(const std::string &chainID, uint32_t height) const {
			std::vector<TransactionPtr> txns;
			std::string sql;

			sql = "SELECT " +
				  _txHash + "," +
				  _buff + "," +
				  _blockHeight + "," +
				  _timestamp + "," +
				  _iso +
				  " FROM " + _tableName + " WHERE " + _blockHeight + " >= ?;";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return txns;
			}

			if (!_sqlite->BindInt(stmt, 1, height)) {
				Log::error("bind args");
			}

			GetSelectedTxns(txns, chainID, stmt);

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return txns;
		}

		std::vector<TransactionPtr> TransactionNormal::GetAll(const std::string &chainID) const {
			std::vector<TransactionPtr> txns;
			std::string sql;

			sql = "SELECT " +
				  _txHash + "," +
				  _buff + "," +
				  _blockHeight + "," +
				  _timestamp + "," +
				  _iso +
				  " FROM " + _tableName + ";";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return txns;
			}

			GetSelectedTxns(txns, chainID, stmt);

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return txns;
		}

		std::vector<TransactionPtr> TransactionNormal::Gets(const std::string &chainID, size_t offset,
															size_t limit, bool asc) const {
			std::vector<TransactionPtr> txns;
			std::string sql, order;

			if (asc) {
				order = " ASC ";
			} else {
				order = " DESC ";
			}

			sql = "SELECT " +
				  _txHash + "," +
				  _buff + "," +
				  _blockHeight + "," +
				  _timestamp + "," +
				  _iso +
				  " FROM " + _tableName + " ORDER BY " + _blockHeight + order + "LIMIT ? OFFSET ?;";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return txns;
			}

			if (!_sqlite->BindInt64(stmt, 1, limit) ||
				!_sqlite->BindInt64(stmt, 2, offset)) {
				Log::error("bind args");
			}

			GetSelectedTxns(txns, chainID, stmt);

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return txns;
		}

		std::vector<TransactionPtr> TransactionNormal::GetTxnBaseOnHash(const std::string &chainID,
																		const std::string &tableName,
																		const std::string &txHashColumnName) const {
			std::vector<TransactionPtr> txns;
			std::string sql;

			sql = "SELECT " + _txHash + "," + _buff + "," + _blockHeight + "," + _timestamp + "," + _iso +
				  " FROM " + _tableName + " WHERE " +
				  _txHash + " IN (SELECT " + txHashColumnName + " FROM " + tableName + " GROUP BY " +
				  txHashColumnName + ");";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			GetSelectedTxns(txns, chainID, stmt);

			int r = sqlite3_finalize(stmt);
			if (SQLITE_OK != r) {
				Log::error("Tx get txn({}) finalize: r = {}, extend code: {}", txns.size(), r, _sqlite->ExtendedEerrCode());
				return {};
			}

			return txns;
		}

		bool TransactionNormal::Update(const std::vector<TransactionPtr> &txns) {
			return DoTransaction([&txns, this]() {
				for (const TransactionPtr &tx : txns) {
					if (!this->_Update(tx))
						return false;
				}
				return true;
			});
		}

		bool TransactionNormal::DeleteByHash(const uint256 &hash) {
			return DoTransaction([&hash, this]() {
				return this->_DeleteByHash(hash);
			});
		}

		bool TransactionNormal::DeleteByHashes(const std::vector<uint256> &hashes) {
			return DoTransaction([&hashes, this]() {
				return this->_DeleteByHashes(hashes);
			});
		}

		TransactionPtr TransactionNormal::SelectByHash(const uint256 &hash, const std::string &chainID) const {
			std::vector<TransactionPtr> txns;
			std::string txHash = hash.GetHex();

			std::string sql;

			sql = "SELECT " +
				  _txHash + "," +
				  _buff + "," +
				  _blockHeight + "," +
				  _timestamp + "," +
				  _iso +
				  " FROM " + _tableName +
				  " WHERE " + _txHash + " = ?;";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return nullptr;
			}

			if (!_sqlite->BindText(stmt, 1, txHash, nullptr)) {
				Log::error("bind args");
			}

			GetSelectedTxns(txns, chainID, stmt);

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx select finalize");
				return nullptr;
			}

			return txns.empty() ? nullptr : txns[0];
		}

		void TransactionNormal::GetSelectedTxns(std::vector<TransactionPtr> &txns, const std::string &chainID,
												sqlite3_stmt *stmt) const {
			while (SQLITE_ROW == _sqlite->Step(stmt)) {
				TransactionPtr tx;
				if (chainID == CHAINID_MAINCHAIN) {
					tx = TransactionPtr(new Transaction());
				} else if (chainID == CHAINID_IDCHAIN || chainID == CHAINID_TOKENCHAIN) {
					tx = TransactionPtr(new IDTransaction());
				}

				uint256 txHash(_sqlite->ColumnText(stmt, 0));

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
				ByteStream stream(pdata, len);

				uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
				uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
				std::string iso = _sqlite->ColumnText(stmt, 4);

				if (iso == ISO_OLD) {
					tx->Deserialize(stream);
					assert(txHash == tx->GetHash());
				} else if (iso == ISO) {
					tx->Deserialize(stream, true);
					tx->SetHash(txHash);
				}

				tx->SetBlockHeight(blockHeight);
				tx->SetTimestamp(timeStamp);

				txns.push_back(tx);
			}
		}

		bool TransactionNormal::ContainHash(const uint256 &hash) const {
			bool contain = false;
			std::string txHash = hash.GetHex();
			std::string sql;

			sql = "SELECT " + _txHash + " FROM " + _tableName + " WHERE " + _txHash + " = ?;";

			sqlite3_stmt *stmt = NULL;
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
				Log::error("Tx contain finalize");
				return false;
			}

			return contain;
		}

		bool TransactionNormal::_Update(const TransactionPtr &txn) {
			std::string hash = txn->GetHash().GetHex();
			uint32_t blockHeight = txn->GetBlockHeight();
			time_t timestamp = txn->GetTimestamp();

			std::string sql = "UPDATE " + _tableName + " SET " + _blockHeight + " = ?, " + _timestamp + " = ? " +
							  " WHERE " + _txHash + " = ?;";

			sqlite3_stmt *stmt = NULL;
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
				Log::error("Tx update finalize");
				return false;
			}

			return true;
		}

		bool TransactionNormal::_DeleteByHashes(const std::vector<uint256> &hashes) {
			for (const uint256 &hash : hashes) {
				if (!_DeleteByHash(hash))
					return false;
			}

			return true;
		}

		bool TransactionNormal::_DeleteByHash(const uint256 &hash) {
			std::string sql;
			std::string hashString = hash.GetHex();

			sql = "DELETE FROM " + _tableName + " WHERE " + _txHash + " = ?;";

			sqlite3_stmt *stmt = NULL;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, hashString, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx delete finalize");
				return false;
			}
			return true;
		}

		bool TransactionNormal::_Puts(const std::vector<TransactionPtr> &txns, bool replace) {
			if (replace) {
				std::string sql = "DELETE FROM " + _tableName + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}" + sql);
					return false;
				}
			}

			for (size_t i = 0; i < txns.size(); ++i) {
				if (!this->_Put(txns[i]))
					return false;
			}

			return true;
		}

	} // namespace ElaWallet
} // namespace Elastos
