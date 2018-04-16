// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <string>
#include <sstream>

#include "TransactionDataStore.h"

namespace Elastos {
	namespace SDK {

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) :
			_sqlite(sqlite) {
			_sqlite->transaction(IMMEDIATE, TX_DATABASE_CREATE, nullptr, nullptr);
		}

		TransactionDataStore::~TransactionDataStore() {
		}

		bool TransactionDataStore::putTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
			std::stringstream ss;

			ss << "insert into " << TX_TABLE_NAME   << "(" <<
				TX_COLUMN_ID    << "," <<
				TX_BUFF         << "," <<
				TX_BLOCK_HEIGHT << "," <<
				TX_TIME_STAMP   << "," <<
				TX_ISO          <<
				") VALUES (?, ?, ?, ?, ?);";

			_sqlite->exec("BEGIN IMMEDIATE;", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return false;
			}

			_sqlite->bindText(stmt, 1, transactionEntity.txHash, nullptr);
			_sqlite->bindBlob(stmt, 2, transactionEntity.buff, nullptr);
			_sqlite->bindInt(stmt, 3, transactionEntity.blockHeight);
			_sqlite->bindInt(stmt, 4, transactionEntity.timeStamp);
			_sqlite->bindText(stmt, 5, iso, nullptr);

			_sqlite->finalize(stmt);

			return _sqlite->exec("COMMIT;", nullptr, nullptr);
		}

		bool TransactionDataStore::deleteAllTransactions(const std::string &iso) {
			std::stringstream ss;

			ss << "delete from " << TX_TABLE_NAME <<
				" where " << TX_ISO << " = '" << iso << "';";

			return _sqlite->transaction(IMMEDIATE, ss.str(), nullptr, nullptr);
		}

		std::vector<TransactionEntity> TransactionDataStore::getAllTransactions(const std::string &iso) const {
			std::vector<TransactionEntity> transactions;

			std::stringstream ss;

			ss << "SELECT " <<
				TX_COLUMN_ID    << ", " <<
				TX_BUFF         << ", " <<
				TX_BLOCK_HEIGHT << ", " <<
				TX_TIME_STAMP   <<
				" FROM " << TX_TABLE_NAME <<
				" WHERE " << TX_ISO << " = '" << iso << "';";

			_sqlite->exec("BEGIN IMMEDIATE;", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return transactions;
			}

			TransactionEntity tx;
			while (SQLITE_ROW == _sqlite->step(stmt)) {
				// id
				tx.txHash = _sqlite->columnText(stmt, 0);

				// block data
				const uint8_t *pdata = (const uint8_t *)_sqlite->columnBlob(stmt, 1);
				size_t len = _sqlite->columnBytes(stmt, 1);
				tx.buff.length = len;
				tx.buff.data = new uint8_t[len];
				memcpy(tx.buff.data, pdata, len);

				// block height
				tx.blockHeight = _sqlite->columnInt(stmt, 2);

				// timestamp
				tx.timeStamp = _sqlite->columnInt(stmt, 3);

				transactions.push_back(tx);
			}

			_sqlite->finalize(stmt);

			_sqlite->exec("COMMIT;", nullptr, nullptr);

			return transactions;
		}

		bool TransactionDataStore::updateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			std::stringstream ss;

			ss << "UPDATE " << TX_TABLE_NAME << " SET " <<
				TX_BUFF         << " = ?, " <<
				TX_BLOCK_HEIGHT << " = ?, " <<
				TX_TIME_STAMP   << " = ? "
				" where " << TX_ISO << " = '" << iso << "'" <<
				" and " << TX_COLUMN_ID << " = '" << txEntity.txHash << "';";

			_sqlite->exec("BEGIN IMMEDIATE;", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return false;
			}

			_sqlite->bindBlob(stmt, 1, txEntity.buff, nullptr);
			_sqlite->bindInt(stmt, 2, txEntity.blockHeight);
			_sqlite->bindInt(stmt, 3, txEntity.timeStamp);

			_sqlite->finalize(stmt);

			return _sqlite->exec("COMMIT;", nullptr, nullptr);
		}

		bool TransactionDataStore::deleteTxByHash(const std::string &iso, const std::string &hash) {
			std::stringstream ss;

			ss << "delete from " << TX_TABLE_NAME <<
				" where " << TX_ISO << " = '" << iso << "'" <<
				" and " << TX_COLUMN_ID << " = '" << hash << "';";

			return _sqlite->transaction(IMMEDIATE, ss.str(), nullptr, nullptr);
		}

	}
}
