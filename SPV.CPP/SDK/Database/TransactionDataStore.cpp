// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <string>
#include <sstream>

#include "TransactionDataStore.h"

namespace Elastos {
	namespace SDK {

		int getTransactionCallBack(void *arg, int column, char **value, char **header) {
			return SQLITE_OK;
		}

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) :
			_sqlite(sqlite) {
			std::string sqlUpper = TX_DATABASE_CREATE;
			std::transform(sqlUpper.begin(), sqlUpper.end(), sqlUpper.begin(), ::toupper);
			_sqlite->execSql(sqlUpper, nullptr, nullptr);
		}

		TransactionDataStore::~TransactionDataStore() {
		}

		bool TransactionDataStore::putTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
			std::stringstream ss;

			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);
#if 0
			ss << "INSET INTO " << TX_TABLE_NAME   << "("
				<< TX_COLUMN_ID                    << ","
				<< TX_BUFF                         << ","
				<< TX_BLOCK_HEIGHT                 << ","
				<< TX_ISO                          << ","
				<< TX_TIME_STAMP                   << ","
				<< ") VALUES ("
				<< "'" << transactionEntity.txHash << "',"
				<< "'" << transactionEntity.buff   << "'," // TODO heropan blob if need to add ''
				<< transactionEntity.blockheight   << ","
				<< "'" << isoUpper                 << "',"
				<< transactionEntity.timestamp     << ");";
#endif

			return _sqlite->execSql(ss.str(), nullptr, nullptr);
		}

		bool TransactionDataStore::deleteAllTransactions(const std::string &iso) {
			std::stringstream ss;
			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);
#if 0
			ss << "DELETE FROM " << TX_TABLE_NAME << " WHERE "
				<< TX_ISO << " = '" << isoUpper << "';";
#endif

			return _sqlite->execSql(ss.str(), nullptr, nullptr);
		}

		std::vector<TransactionEntity> TransactionDataStore::getAllTransactions(const std::string &iso) const {
			std::vector<TransactionEntity> transactions;

			std::stringstream ss;
			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);

#if 0
			ss << "SELECT * FROM " << TX_TABLE_NAME << " WHERE "
				<< TX_ISO << " = '" << isoUpper << "';";
#endif

			_sqlite->execSql(ss.str(), getTransactionCallBack, (void*)&transactions);

			return transactions;
		}

		bool TransactionDataStore::updateTransaction(const std::string &iso, const TransactionEntity &transactionEntity) {
			std::stringstream ss;
			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);

#if 0
			ss << "UPDATE " << TX_TABLE_NAME << " SET "
				<< TX_COLUMN_ID << " = " <<
				<< TX_BUFF                       << ","
				<< TX_BLOCK_HEIGHT               << ","
				<< TX_ISO                        << ","
				<< TX_TIME_STAMP                 << ",";
#endif

			return _sqlite->execSql(ss.str(), nullptr, nullptr);
		}

		bool TransactionDataStore::deleteTxByHash(const std::string &iso, const std::string &hash) {
			std::stringstream ss;
			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);

			return _sqlite->execSql(ss.str(), nullptr, nullptr);
		}

	}
}
