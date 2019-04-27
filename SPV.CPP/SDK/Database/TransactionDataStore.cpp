// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionDataStore.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>

#include <string>
#include <string>
#include <sstream>

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

		bool TransactionDataStore::PutTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
			TransactionEntity txEntity;

			if (SelectTxByHash(iso, transactionEntity.txHash, txEntity)) {
				return DoTransaction([&iso, &transactionEntity, this]() {
					std::stringstream ss;

					ss << "UPDATE " << TX_TABLE_NAME << " SET "
					   << TX_BUFF << " = ?, "
					   << TX_BLOCK_HEIGHT << " = ?, "
					   << TX_TIME_STAMP << " = ?, "
					   << TX_REMARK << " = ? "
					   << " WHERE " << TX_COLUMN_ID << " = '" << transactionEntity.txHash << "';";

					sqlite3_stmt *stmt;
					ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + ss.str());

					_sqlite->BindBlob(stmt, 1, transactionEntity.buff, nullptr);
					_sqlite->BindInt(stmt, 2, transactionEntity.blockHeight);
					_sqlite->BindInt(stmt, 3, transactionEntity.timeStamp);
					_sqlite->BindText(stmt, 4, transactionEntity.remark, nullptr);

					_sqlite->Step(stmt);

					_sqlite->Finalize(stmt);
				});
			}

			return DoTransaction([&iso, &transactionEntity, this]() {
				std::stringstream ss;

				ss << "INSERT INTO " << TX_TABLE_NAME << "("
				   << TX_COLUMN_ID << ","
				   << TX_BUFF << ","
				   << TX_BLOCK_HEIGHT << ","
				   << TX_TIME_STAMP << ","
				   << TX_REMARK << ","
				   << TX_ASSETID << ","
				   << TX_ISO
				   << ") VALUES (?, ?, ?, ?, ?, ?, ?);";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				_sqlite->BindText(stmt, 1, transactionEntity.txHash, nullptr);
				_sqlite->BindBlob(stmt, 2, transactionEntity.buff, nullptr);
				_sqlite->BindInt(stmt, 3, transactionEntity.blockHeight);
				_sqlite->BindInt(stmt, 4, transactionEntity.timeStamp);
				_sqlite->BindText(stmt, 5, transactionEntity.remark, nullptr);
				_sqlite->BindText(stmt, 6, "", nullptr);
				_sqlite->BindText(stmt, 7, iso, nullptr);

				_sqlite->Step(stmt);

				_sqlite->Finalize(stmt);
			});

		}

		bool TransactionDataStore::DeleteAllTransactions(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << TX_TABLE_NAME << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		size_t TransactionDataStore::GetAllTransactionsCount(const std::string &iso) const {
			size_t count = 0;

			DoTransaction([&iso, &count, this]() {
				std::stringstream ss;

				ss << "SELECT " <<
				   " COUNT(" << TX_COLUMN_ID << ") AS nums " <<
				   " FROM " << TX_TABLE_NAME << ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}

				_sqlite->Finalize(stmt);
			});

			return count;
		}

		std::vector<TransactionEntity> TransactionDataStore::GetAllTransactions(const std::string &iso) const {
			std::vector<TransactionEntity> transactions;

			DoTransaction([&iso, &transactions, this]() {
				std::stringstream ss;

				ss << "SELECT "
				   << TX_COLUMN_ID << ", "
				   << TX_BUFF << ", "
				   << TX_BLOCK_HEIGHT << ", "
				   << TX_TIME_STAMP << ", "
				   << TX_REMARK
				   << " FROM " << TX_TABLE_NAME << ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				TransactionEntity tx;
				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					tx.txHash = _sqlite->ColumnText(stmt, 0);

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);

					tx.buff.assign(pdata, pdata + len);
					tx.blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					tx.timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
					tx.remark = _sqlite->ColumnText(stmt, 4);

					transactions.push_back(tx);
				}

				_sqlite->Finalize(stmt);
			});

			return transactions;
		}

		bool TransactionDataStore::UpdateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			return DoTransaction([&iso, &txEntity, this]() {
				std::stringstream ss;

				ss << "UPDATE " << TX_TABLE_NAME << " SET "
				   << TX_BLOCK_HEIGHT << " = ?, "
				   << TX_TIME_STAMP << " = ? "
				   << " WHERE " << TX_COLUMN_ID << " = '" << txEntity.txHash << "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				_sqlite->BindInt(stmt, 1, txEntity.blockHeight);
				_sqlite->BindInt(stmt, 2, txEntity.timeStamp);

				_sqlite->Step(stmt);

				_sqlite->Finalize(stmt);
			});
		}

		bool TransactionDataStore::DeleteTxByHash(const std::string &iso, const std::string &hash) {
			return DoTransaction([&iso, &hash, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
				   << TX_TABLE_NAME
				   << " WHERE " << TX_COLUMN_ID << " = '" << hash << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		bool TransactionDataStore::SelectTxByHash(const std::string &iso, const std::string &hash,
												  TransactionEntity &txEntity) const {
			bool found = false;

			DoTransaction([&iso, &hash, &txEntity, &found, this]() {
				std::stringstream ss;

				ss << "SELECT "
				   << TX_BUFF << ", "
				   << TX_BLOCK_HEIGHT << ", "
				   << TX_TIME_STAMP << ", "
				   << TX_REMARK
				   << " FROM " << TX_TABLE_NAME
				   << " WHERE " << TX_COLUMN_ID << " = '" << hash << "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					found = true;

					txEntity.txHash = hash;

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 0);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 0);

					txEntity.buff.assign(pdata, pdata + len);
					txEntity.blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 1);
					txEntity.timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 2);
					txEntity.remark = _sqlite->ColumnText(stmt, 3);
				}
			});

			return found;
		}

	}
}
