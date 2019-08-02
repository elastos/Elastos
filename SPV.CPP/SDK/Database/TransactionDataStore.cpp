// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionDataStore.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <string>
#include <string>
#include <sstream>
#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) :
			TableBase(sqlite) {
			InitializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			InitializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::~TransactionDataStore() {
		}

		void TransactionDataStore::PutTransactionInternal(const std::string &iso, const TransactionPtr &tx) {
			std::string sql, txHash;

			sql = "INSERT INTO " + TX_TABLE_NAME + "("
				  + TX_COLUMN_ID + ","
				  + TX_BUFF + ","
				  + TX_BLOCK_HEIGHT + ","
				  + TX_TIME_STAMP + ","
				  + TX_REMARK + ","
				  + TX_ASSETID + ","
				  + TX_ISO
				  + ") VALUES (?, ?, ?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + sql);
			ByteStream stream;
			tx->Serialize(stream, true);

			txHash = tx->GetHash().GetHex();
			_sqlite->BindText(stmt, 1, txHash, nullptr);
			_sqlite->BindBlob(stmt, 2, stream.GetBytes(), nullptr);
			_sqlite->BindInt(stmt, 3, tx->GetBlockHeight());
			_sqlite->BindInt64(stmt, 4, tx->GetTimestamp());
			_sqlite->BindText(stmt, 5, "", nullptr);
			_sqlite->BindText(stmt, 6, "", nullptr);
			_sqlite->BindText(stmt, 7, iso, nullptr);

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);
		}

		bool TransactionDataStore::PutTransaction(const std::string &iso, const TransactionPtr &tx) {
#ifdef SPDLOG_DEBUG_ON
			std::string txHash = tx->GetHash().GetHex();
			if (SelectTxByHash(txHash)) {
				Log::error("should not put in existed tx {}", tx->GetHash().GetHex());
				return false;
			}
#endif

			return DoTransaction([&iso, &tx, this]() {
				this->PutTransactionInternal(iso, tx);
			});
		}

		bool TransactionDataStore::PutTransactions(const std::string &iso, const std::vector<TransactionPtr> &txns) {
			if (txns.empty())
				return true;

			return DoTransaction([&iso, &txns, this]() {
				for (size_t i = 0; i < txns.size(); ++i) {
					this->PutTransactionInternal(iso, txns[i]);
				}
			});
		}

		bool TransactionDataStore::DeleteAllTransactions() {
			return DoTransaction([this]() {
				std::string sql;

				sql = "DELETE FROM " + TX_TABLE_NAME + ";";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		size_t TransactionDataStore::GetAllTransactionsCount() const {
			size_t count = 0;

			DoTransaction([&count, this]() {
				std::string sql;

				sql = std::string("SELECT ") + " COUNT(" + TX_COLUMN_ID + ") AS nums " +
					  " FROM " + TX_TABLE_NAME + ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}

				_sqlite->Finalize(stmt);
			});

			return count;
		}

		std::vector<TransactionPtr> TransactionDataStore::GetAllTransactions() const {
			std::vector<TransactionPtr> txns;
			DoTransaction([&txns, this]() {
				std::string sql;

				sql = "SELECT "
					  + TX_COLUMN_ID + ","
					  + TX_BUFF + ","
					  + TX_BLOCK_HEIGHT + ","
					  + TX_TIME_STAMP + ","
					  + TX_ISO
					  + " FROM " + TX_TABLE_NAME + ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					TransactionPtr tx(new Transaction());

					uint256 txHash(_sqlite->ColumnText(stmt, 0));

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
					ByteStream stream(pdata, len);

					uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
					std::string iso = _sqlite->ColumnText(stmt, 4);

					if (iso == "ela") {
						tx->Deserialize(stream);
						assert(txHash == tx->GetHash());
					} else if (iso == "ela1") {
						tx->Deserialize(stream, true);
						tx->SetHash(txHash);
					}

					tx->SetBlockHeight(blockHeight);
					tx->SetTimestamp(timeStamp);

					txns.push_back(tx);
				}

				_sqlite->Finalize(stmt);
			});
			return txns;
		}

		bool TransactionDataStore::UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight,
													 time_t timestamp) {
			return DoTransaction([&hashes, &blockHeight, &timestamp, this]() {
				std::string sql;

				for (size_t i = 0; i < hashes.size(); ++i) {
					sql = "UPDATE " + TX_TABLE_NAME + " SET "
						  + TX_BLOCK_HEIGHT + " = ?, "
						  + TX_TIME_STAMP + " = ? "
						  + " WHERE " + TX_COLUMN_ID + " = '" + hashes[i].GetHex() + "';";

					sqlite3_stmt *stmt;
					ErrorChecker::CheckLogic(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

					ErrorChecker::CheckLogic(!_sqlite->BindInt(stmt, 1, blockHeight), Error::SqliteError, "bindint");
					ErrorChecker::CheckLogic(!_sqlite->BindInt64(stmt, 2, timestamp), Error::SqliteError, "bindint64");

					_sqlite->Step(stmt);

					ErrorChecker::CheckLogic(!_sqlite->Finalize(stmt), Error::SqliteError, "finalize");
				}
			});
		}

		bool TransactionDataStore::DeleteTxByHash(const uint256 &hash) {
			return DoTransaction([&hash, this]() {
				std::string sql;

				sql = "DELETE FROM " + TX_TABLE_NAME + " WHERE " + TX_COLUMN_ID + " = '" + hash.GetHex() + "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		bool TransactionDataStore::DeleteTxByHashes(const std::vector<uint256> &hashes) {
			if (hashes.empty())
				return true;

			return DoTransaction([&hashes, this]() {
				std::string sql;
				for (size_t i = 0; i < hashes.size(); ++i) {
					sql = "DELETE FROM " + TX_TABLE_NAME + " WHERE " + TX_COLUMN_ID + " = '" + hashes[i].GetHex() + "';";

					ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
												 "Exec sql " + sql);
				}
			});
		}

		void TransactionDataStore::flush() {
			_sqlite->flush();
		}

		TransactionPtr TransactionDataStore::SelectTxByHash(const std::string &hash) const {
			TransactionPtr tx = nullptr;

			DoTransaction([&hash, &tx, this]() {
				std::string sql;

				sql = "SELECT " + TX_COLUMN_ID + "," + TX_BUFF + "," + TX_BLOCK_HEIGHT + "," + TX_TIME_STAMP + "," + TX_ISO +
					  " FROM " + TX_TABLE_NAME +
					  " WHERE " + TX_COLUMN_ID + " = '" + hash + "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					TransactionPtr t(new Transaction());

					uint256 txHash(_sqlite->ColumnText(stmt, 0));

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
					ByteStream stream(pdata, len);

					uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
					std::string iso = _sqlite->ColumnText(stmt, 4);

					if (iso == "ela") {
						t->Deserialize(stream);
						assert(txHash == tx->GetHash());
					} else if (iso == "ela1") {
						t->Deserialize(stream, true);
						t->SetHash(txHash);
					}

					t->SetBlockHeight(blockHeight);
					t->SetTimestamp(timeStamp);
					tx = t;
				}
			});

			return tx;
		}

	}
}
