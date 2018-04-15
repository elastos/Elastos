// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include "DatabaseManager.h"

namespace Elastos {
	namespace SDK {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_transactionDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite) {

		}

		DatabaseManager::DatabaseManager() :
			DatabaseManager("wallet.db") {

		}

		DatabaseManager::~DatabaseManager() {

		}

		bool DatabaseManager::putTransaction(const std::string &iso, const TransactionEntity &tx) {
			return _transactionDataStore.putTransaction(iso, tx);
		}

	}
}
