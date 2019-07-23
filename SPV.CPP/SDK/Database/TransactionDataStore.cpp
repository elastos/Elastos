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

		bool TransactionDataStore::PutTransaction(const std::string &iso, const TransactionPtr &transaction) {
			TransactionPtr tx;

			if (SelectTxByHash(iso, transaction->GetHash().GetHex(), tx)) {
				return DoTransaction([&iso, &transaction, this]() {
					std::string sql;

					sql = "UPDATE " + TX_TABLE_NAME + " SET "
					   + TX_BUFF + " = ?, "
					   + TX_BLOCK_HEIGHT + " = ?, "
					   + TX_TIME_STAMP + " = ? "
					   + " WHERE " + TX_COLUMN_ID + " = '" + transaction->GetHash().GetHex() + "';";

					sqlite3_stmt *stmt;
					ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + sql);

					ByteStream stream;
					transaction->Serialize(stream);
					_sqlite->BindBlob(stmt, 1, stream.GetBytes(), nullptr);
					_sqlite->BindInt(stmt, 2, transaction->GetBlockHeight());
					_sqlite->BindInt(stmt, 3, transaction->GetTimestamp());

					_sqlite->Step(stmt);

					_sqlite->Finalize(stmt);
				});
			}

			return DoTransaction([&iso, &transaction, this]() {
				std::string sql;

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
				transaction->Serialize(stream);

				_sqlite->BindText(stmt, 1, transaction->GetHash().GetHex(), nullptr);
				_sqlite->BindBlob(stmt, 2, stream.GetBytes(), nullptr);
				_sqlite->BindInt(stmt, 3, transaction->GetBlockHeight());
				_sqlite->BindInt(stmt, 4, transaction->GetTimestamp());
				_sqlite->BindText(stmt, 5, "", nullptr);
				_sqlite->BindText(stmt, 6, "", nullptr);
				_sqlite->BindText(stmt, 7, iso, nullptr);

				_sqlite->Step(stmt);

				_sqlite->Finalize(stmt);
			});

		}

		bool TransactionDataStore::DeleteAllTransactions(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::string sql;

				sql = "DELETE FROM " + TX_TABLE_NAME + ";";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		size_t TransactionDataStore::GetAllTransactionsCount(const std::string &iso) const {
			size_t count = 0;

			DoTransaction([&iso, &count, this]() {
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

		std::vector<TransactionPtr> TransactionDataStore::GetAllTransactions(const std::string &iso) const {
			std::vector<TransactionPtr> transactions;

			DoTransaction([&iso, &transactions, this]() {
				std::string sql;

				sql = "SELECT "
				   + TX_COLUMN_ID + ", "
				   + TX_BUFF + ", "
				   + TX_BLOCK_HEIGHT + ", "
				   + TX_TIME_STAMP
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
					tx->Deserialize(stream);
					uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);

					tx->SetBlockHeight(blockHeight);
					tx->SetTimestamp(timeStamp);

					ErrorChecker::CheckCondition(txHash != tx->GetHash(), Error::InvalidTransaction, "tx data error");

					transactions.push_back(tx);

				}

				_sqlite->Finalize(stmt);
			});

			return transactions;
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
					ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + sql);

					_sqlite->BindInt(stmt, 1, blockHeight);
					_sqlite->BindInt64(stmt, 2, timestamp);

					_sqlite->Step(stmt);

					_sqlite->Finalize(stmt);
				}
			});
		}

		bool TransactionDataStore::DeleteTxByHash(const std::string &iso, const std::string &hash) {
			return DoTransaction([&iso, &hash, this]() {
				std::string sql;

				sql = "DELETE FROM "
				   + TX_TABLE_NAME + " WHERE " + TX_COLUMN_ID + " = '" + hash + "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		bool TransactionDataStore::DeleteTxByHashes(const std::vector<std::string> &hashes) {
			return DoTransaction([&hashes, this]() {
				std::string sql;
				for (size_t i = 0; i < hashes.size(); ++i) {
					sql = "DELETE FROM " + TX_TABLE_NAME + " WHERE " + TX_COLUMN_ID + " = '" + hashes[i] + "';";

					ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
												 "Exec sql " + sql);
				}
			});
		}

		bool TransactionDataStore::SelectTxByHash(const std::string &iso, const std::string &hash,
		                                          TransactionPtr &transactionPtr) const {
			bool found = false;

			DoTransaction([&iso, &hash, &transactionPtr, &found, this]() {
				std::string sql;

				sql = "SELECT "
				   + TX_BUFF + ", "
				   + TX_BLOCK_HEIGHT + ", "
				   + TX_TIME_STAMP
				   + " FROM " + TX_TABLE_NAME
				   + " WHERE " + TX_COLUMN_ID + " = '" + hash + "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					found = true;

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 0);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 0);

					ByteStream stream(pdata, len);
					transactionPtr->Deserialize(stream);

					uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 1);
					uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 2);

					transactionPtr->SetBlockHeight(blockHeight);
					transactionPtr->SetTimestamp(timeStamp);

					ErrorChecker::CheckCondition(hash == transactionPtr->GetHash().GetHex(), Error::InvalidTransaction,
					                             "tx data error");
				}
			});

			return found;
		}

	}
}
