// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <sstream>

#include "PreCompiled.h"
#include "TransactionDataStore.h"

namespace Elastos {
	namespace SDK {

		int putTransactionCallBack(void* arg, int column, char** value, char** header) {
			return SQLITE_OK;
		}

		TransactionDataStore::TransactionDataStore() {

		}

		TransactionDataStore::~TransactionDataStore() {

		}

		bool TransactionDataStore::putTransaction(Sqlite& sqlite, std::string& iso, BRTransactionEntity &transactionEntity) {

			std::stringstream ss;

			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);
			ss << "INSET INTO " << TX_TABLE_NAME << "(" <<
				TX_COLUMN_ID                  << "," <<
				TX_BUFF                       << "," <<
				TX_BLOCK_HEIGHT               << "," <<
				TX_ISO                        << "," <<
				TX_TIME_STAMP                 << "," <<
				") VALUES("                   <<
				transactionEntity.txHash      << "," <<
				transactionEntity.buff        << "," <<
				transactionEntity.blockheight << "," <<
				isoUpper                      << "," <<
				transactionEntity.timestamp   << ");";

			return sqlite.execSql(ss.str(), putTransactionCallBack, NULL);
		}




	}
}