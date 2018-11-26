// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionDataStore.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>

#include <string>
#include <string>
#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) :
				TableBase(sqlite) {
			initializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite) :
				TableBase(type, sqlite) {
			initializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::~TransactionDataStore() {
		}

		bool TransactionDataStore::putTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
			TransactionEntity txEntity;

			if (selectTxByHash(iso, transactionEntity.txHash, txEntity)) {
				return doTransaction([&iso, &transactionEntity, this]() {
					std::stringstream ss;

					ss << "UPDATE " << TX_TABLE_NAME << " SET "
						<< TX_BUFF         << " = ?, "
						<< TX_BLOCK_HEIGHT << " = ?, "
						<< TX_TIME_STAMP   << " = ?, "
						<< TX_REMARK       << " = ?, "
						<< TX_ASSETID      << " = ? "
						<< " WHERE " << TX_ISO << " = '" << iso << "'"
						<< " AND " << TX_COLUMN_ID << " = '" << transactionEntity.txHash << "';";

					sqlite3_stmt *stmt;
					ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + ss.str());

#ifdef NDEBUG
					_sqlite->bindBlob(stmt, 1, transactionEntity.buff, nullptr);
#else
					std::string str = Utils::encodeHex(transactionEntity.buff);
					CMBlock bytes;
					bytes.SetMemFixed((const uint8_t *) str.c_str(), str.length());
					_sqlite->bindBlob(stmt, 1, bytes, nullptr);
#endif
					_sqlite->bindInt(stmt, 2, transactionEntity.blockHeight);
					_sqlite->bindInt(stmt, 3, transactionEntity.timeStamp);
					_sqlite->bindText(stmt, 4, transactionEntity.remark, nullptr);
					_sqlite->bindText(stmt, 5, transactionEntity.assetID, nullptr);

					_sqlite->step(stmt);

					_sqlite->finalize(stmt);
				});
			}

			return doTransaction([&iso, &transactionEntity, this]() {
				std::stringstream ss;

				ss << "INSERT INTO " << TX_TABLE_NAME << "("
					<< TX_COLUMN_ID    << ","
					<< TX_BUFF         << ","
					<< TX_BLOCK_HEIGHT << ","
					<< TX_TIME_STAMP   << ","
					<< TX_REMARK       << ","
					<< TX_ASSETID      << ","
					<< TX_ISO
					<< ") VALUES (?, ?, ?, ?, ?, ?, ?);";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				_sqlite->bindText(stmt, 1, transactionEntity.txHash, nullptr);
#ifdef NDEBUG
				_sqlite->bindBlob(stmt, 2, transactionEntity.buff, nullptr);
#else
				std::string str = Utils::encodeHex(transactionEntity.buff);
				CMBlock bytes;
				bytes.SetMemFixed((const uint8_t *) str.c_str(), str.length());
				_sqlite->bindBlob(stmt, 2, bytes, nullptr);
#endif
				_sqlite->bindInt(stmt, 3, transactionEntity.blockHeight);
				_sqlite->bindInt(stmt, 4, transactionEntity.timeStamp);
				_sqlite->bindText(stmt, 5, transactionEntity.remark, nullptr);
				_sqlite->bindText(stmt, 6, transactionEntity.assetID, nullptr);
				_sqlite->bindText(stmt, 7, iso, nullptr);

				_sqlite->step(stmt);

				_sqlite->finalize(stmt);
			});

		}

		bool TransactionDataStore::deleteAllTransactions(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << TX_TABLE_NAME <<
				   " WHERE " << TX_ISO << " = '" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		size_t TransactionDataStore::getAllTransactionsCount(const std::string &iso) const {
			size_t count = 0;

			doTransaction([&iso, &count, this]() {
				std::stringstream ss;

				ss << "SELECT " <<
				   " COUNT(" << TX_COLUMN_ID << ") AS nums " <<
				   " FROM " << TX_TABLE_NAME << ";";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					count = (uint32_t) _sqlite->columnInt(stmt, 0);
				}

				_sqlite->finalize(stmt);
			});

			return count;
		}

		std::vector<TransactionEntity> TransactionDataStore::getAllTransactions(const std::string &iso) const {
			std::vector<TransactionEntity> transactions;

			doTransaction([&iso, &transactions, this]() {
				std::stringstream ss;

				ss << "SELECT "
					<< TX_COLUMN_ID    << ", "
					<< TX_BUFF         << ", "
					<< TX_BLOCK_HEIGHT << ", "
					<< TX_TIME_STAMP   << ", "
					<< TX_ASSETID      << ", "
					<< TX_REMARK
					<< " FROM " << TX_TABLE_NAME
					<< " WHERE " << TX_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				TransactionEntity tx;
				while (SQLITE_ROW == _sqlite->step(stmt)) {
					tx.txHash = _sqlite->columnText(stmt, 0);

					const uint8_t *pdata = (const uint8_t *) _sqlite->columnBlob(stmt, 1);
					size_t len = (size_t) _sqlite->columnBytes(stmt, 1);

#ifdef NDEBUG
					CMBlock buff;
					buff.Resize(len);
					memcpy(buff, pdata, len);
					tx.buff = buff;
#else
					std::string str((char *) pdata, len);
					tx.buff = Utils::decodeHex(str);
#endif

					tx.blockHeight = (uint32_t) _sqlite->columnInt(stmt, 2);
					tx.timeStamp = (uint32_t) _sqlite->columnInt(stmt, 3);
					tx.assetID = _sqlite->columnText(stmt, 4);
					tx.remark = _sqlite->columnText(stmt, 5);

					transactions.push_back(tx);
				}

				_sqlite->finalize(stmt);
			});

			return transactions;
		}

		bool TransactionDataStore::updateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			return doTransaction([&iso, &txEntity, this]() {
				std::stringstream ss;

				ss << "UPDATE " << TX_TABLE_NAME << " SET "
					<< TX_BLOCK_HEIGHT << " = ?, "
					<< TX_TIME_STAMP   << " = ?, "
					<< TX_ASSETID      << " = ? "
					<< " WHERE " << TX_ISO << " = '" << iso << "'"
					<< " AND " << TX_COLUMN_ID << " = '" << txEntity.txHash << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				_sqlite->bindInt(stmt, 1, txEntity.blockHeight);
				_sqlite->bindInt(stmt, 2, txEntity.timeStamp);
				_sqlite->bindText(stmt, 3, txEntity.assetID, nullptr);

				_sqlite->step(stmt);

				_sqlite->finalize(stmt);
			});
		}

		bool TransactionDataStore::deleteTxByHash(const std::string &iso, const std::string &hash) {
			return doTransaction([&iso, &hash, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
					<< TX_TABLE_NAME
					<< " WHERE " << TX_ISO << " = '" << iso << "'"
					<< " AND " << TX_COLUMN_ID << " = '" << hash << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		bool TransactionDataStore::selectTxByHash(const std::string &iso, const std::string &hash,
												  TransactionEntity &txEntity) const {
			bool found = false;

			doTransaction([&iso, &hash, &txEntity, &found, this]() {
				std::stringstream ss;

				ss << "SELECT "
					<< TX_BUFF         << ", "
					<< TX_BLOCK_HEIGHT << ", "
					<< TX_TIME_STAMP   << ", "
					<< TX_ASSETID      << ", "
					<< TX_REMARK
					<< " FROM " << TX_TABLE_NAME
					<< " WHERE " << TX_ISO << " = '" << iso << "'"
					<< " AND " << TX_COLUMN_ID << " = '" << hash << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					found = true;

					txEntity.txHash = hash;

					const uint8_t *pdata = (const uint8_t *) _sqlite->columnBlob(stmt, 0);
					size_t len = (size_t) _sqlite->columnBytes(stmt, 0);

#ifdef NDEBUG
					CMBlock buff;
					buff.Resize(len);
					memcpy(buff, pdata, len);
					txEntity.buff = buff;
#else
					std::string str((char *) pdata, len);
					txEntity.buff = Utils::decodeHex(str);
#endif

					txEntity.blockHeight = (uint32_t) _sqlite->columnInt(stmt, 1);
					txEntity.timeStamp = (uint32_t) _sqlite->columnInt(stmt, 2);
					txEntity.assetID = _sqlite->columnText(stmt, 3);
					txEntity.remark = _sqlite->columnText(stmt, 4);
				}
			});

			return found;
		}

	}
}
