// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "TransactionDataStore.h"
#include "PeerDataStore.h"
#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		class DatabaseManager {
		public:
			DatabaseManager(boost::filesystem::path path);
			DatabaseManager();
			~DatabaseManager();

			bool putTransaction(std::string iso, BRTransactionEntity& transactionEntity);
		private:
			TransactionDataStore _dataStoreTransaction;
			PeerDataStore        _dataStorePeer;
			Sqlite               _sqlite;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
