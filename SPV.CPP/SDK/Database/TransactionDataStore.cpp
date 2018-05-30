// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <string>
#include <string>
#include <sstream>

#include "TransactionDataStore.h"

namespace Elastos {
	namespace SDK {

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(EXCLUSIVE) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			_sqlite->transaction(_txType, TX_DATABASE_CREATE, nullptr, nullptr);
		}

		TransactionDataStore::TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(type) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			_sqlite->transaction(_txType, TX_DATABASE_CREATE, nullptr, nullptr);
		}

		TransactionDataStore::~TransactionDataStore() {
		}

		bool TransactionDataStore::putTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			std::stringstream ss;

			ss << "INSERT INTO " << TX_TABLE_NAME   << "(" <<
				TX_COLUMN_ID    << "," <<
				TX_BUFF         << "," <<
				TX_BLOCK_HEIGHT << "," <<
				TX_TIME_STAMP   << "," <<
				TX_ISO          <<
				") VALUES (?, ?, ?, ?, ?);";

			_sqlite->beginTransaction(_txType);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				_sqlite->endTransaction();
				return false;
			}

			_sqlite->bindText(stmt, 1, transactionEntity.txHash, nullptr);
			_sqlite->bindBlob(stmt, 2, transactionEntity.buff, nullptr);
			_sqlite->bindInt(stmt, 3, transactionEntity.blockHeight);
			_sqlite->bindInt(stmt, 4, transactionEntity.timeStamp);
			_sqlite->bindText(stmt, 5, iso, nullptr);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return _sqlite->endTransaction();
		}

		bool TransactionDataStore::deleteAllTransactions(const std::string &iso) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			std::stringstream ss;

			ss << "DELETE FROM " << TX_TABLE_NAME <<
				" WHERE " << TX_ISO << " = '" << iso << "';";

			return _sqlite->transaction(_txType, ss.str(), nullptr, nullptr);
		}

		std::vector<TransactionEntity> TransactionDataStore::getAllTransactions(const std::string &iso) const {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			std::vector<TransactionEntity> transactions;

			std::stringstream ss;

			ss << "SELECT " <<
				TX_COLUMN_ID    << ", " <<
				TX_BUFF         << ", " <<
				TX_BLOCK_HEIGHT << ", " <<
				TX_TIME_STAMP   <<
				" FROM " << TX_TABLE_NAME <<
				" WHERE " << TX_ISO << " = '" << iso << "';";

			_sqlite->beginTransaction(_txType);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				_sqlite->endTransaction();
				return transactions;
			}

			TransactionEntity tx;
			while (SQLITE_ROW == _sqlite->step(stmt)) {
				CMBlock buff;
				// id
				tx.txHash = _sqlite->columnText(stmt, 0);

				// block data
				const uint8_t *pdata = (const uint8_t *)_sqlite->columnBlob(stmt, 1);
				size_t len = _sqlite->columnBytes(stmt, 1);

				buff.Resize(len);
				memcpy(buff, pdata, len);
				tx.buff = buff;

				// block height
				tx.blockHeight = _sqlite->columnInt(stmt, 2);

				// timestamp
				tx.timeStamp = _sqlite->columnInt(stmt, 3);

				transactions.push_back(tx);
			}

			_sqlite->finalize(stmt);
			_sqlite->endTransaction();

			return transactions;
		}

		bool TransactionDataStore::updateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			std::stringstream ss;

			ss << "UPDATE " << TX_TABLE_NAME << " SET " <<
				TX_BLOCK_HEIGHT << " = ?, " <<
				TX_TIME_STAMP   << " = ? "
				" WHERE " << TX_ISO << " = '" << iso << "'" <<
				" AND " << TX_COLUMN_ID << " = '" << txEntity.txHash << "';";

			_sqlite->beginTransaction(_txType);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				_sqlite->endTransaction();
				return false;
			}

			_sqlite->bindInt(stmt, 1, txEntity.blockHeight);
			_sqlite->bindInt(stmt, 2, txEntity.timeStamp);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return _sqlite->endTransaction();
		}

		bool TransactionDataStore::deleteTxByHash(const std::string &iso, const std::string &hash) {
#ifdef SQLITE_MUTEX_LOCK_ON
			boost::mutex::scoped_lock lock(_lockMutex);
#endif
			std::stringstream ss;

			ss << "DELETE FROM " << TX_TABLE_NAME <<
				" WHERE " << TX_ISO << " = '" << iso << "'" <<
				" AND " << TX_COLUMN_ID << " = '" << hash << "';";

			return _sqlite->transaction(_txType, ss.str(), nullptr, nullptr);
		}

	}
}
