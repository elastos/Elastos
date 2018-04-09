// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include "DatabaseManager.h"

namespace Elastos {
	namespace SDK {

		DatabaseManager::DatabaseManager(boost::filesystem::path path) :
			_sqlite(path),
			_dataStorePeer(),
			_dataStoreTransaction() {

		}

		DatabaseManager::DatabaseManager() :
			DatabaseManager("wallet.db") {

		}

		DatabaseManager::~DatabaseManager() {

		}

		bool DatabaseManager::putTransaction(std::string iso, BRTransactionEntity& transactionEntity) {
			if (!_sqlite.isValid()) {
				return false;
			}

			return _dataStoreTransaction.putTransaction(_sqlite, iso, transactionEntity);
		}


	}
}