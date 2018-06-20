// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"

#include <string>
#include <string>
#include <sstream>
#include <SDK/Common/Log.h>

#include "TransactionDataStore.h"

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
			return doTransaction([&iso, &transactionEntity, this]() {
				std::stringstream ss;

				ss << "INSERT INTO " << TX_TABLE_NAME   << "(" <<
				   TX_COLUMN_ID    << "," <<
				   TX_BUFF         << "," <<
				   TX_BLOCK_HEIGHT << "," <<
				   TX_TIME_STAMP   << "," <<
				   TX_ISO          <<
				   ") VALUES (?, ?, ?, ?, ?);";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					Log::getLogger()->error(ess.str());
					throw std::logic_error(ess.str());
				}

				_sqlite->bindText(stmt, 1, transactionEntity.txHash, nullptr);
				_sqlite->bindBlob(stmt, 2, transactionEntity.buff, nullptr);
				_sqlite->bindInt(stmt, 3, transactionEntity.blockHeight);
				_sqlite->bindInt(stmt, 4, transactionEntity.timeStamp);
				_sqlite->bindText(stmt, 5, iso, nullptr);

				_sqlite->step(stmt);

				_sqlite->finalize(stmt);
			});

		}

		bool TransactionDataStore::deleteAllTransactions(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << TX_TABLE_NAME <<
				   " WHERE " << TX_ISO << " = '" << iso << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		std::vector<TransactionEntity> TransactionDataStore::getAllTransactions(const std::string &iso) const {
			std::vector<TransactionEntity> transactions;

			doTransaction([&iso, &transactions, this]() {
				std::stringstream ss;

				ss << "SELECT " <<
				   TX_COLUMN_ID    << ", " <<
				   TX_BUFF         << ", " <<
				   TX_BLOCK_HEIGHT << ", " <<
				   TX_TIME_STAMP   <<
				   " FROM " << TX_TABLE_NAME <<
				   " WHERE " << TX_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
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
			});

			return transactions;
		}

		bool TransactionDataStore::updateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			return doTransaction([&iso, &txEntity, this]() {
				std::stringstream ss;

				ss << "UPDATE " << TX_TABLE_NAME << " SET " <<
					TX_BLOCK_HEIGHT << " = ?, " <<
					TX_TIME_STAMP   << " = ? "
					" WHERE " << TX_ISO << " = '" << iso << "'" <<
					" AND " << TX_COLUMN_ID << " = '" << txEntity.txHash << "';";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}

				_sqlite->bindInt(stmt, 1, txEntity.blockHeight);
				_sqlite->bindInt(stmt, 2, txEntity.timeStamp);

				_sqlite->step(stmt);

				_sqlite->finalize(stmt);
			});
		}

		bool TransactionDataStore::deleteTxByHash(const std::string &iso, const std::string &hash) {
			return doTransaction([&iso, &hash, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << TX_TABLE_NAME <<
				   " WHERE " << TX_ISO << " = '" << iso << "'" <<
				   " AND " << TX_COLUMN_ID << " = '" << hash << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

	}
}
