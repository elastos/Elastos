// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "MerkleBlockDataSource.h"
#include "TransactionDataStore.h"
#include "PeerDataSource.h"
#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);
			DatabaseManager();
			~DatabaseManager();

			bool putTransaction(const std::string &iso, const TransactionEntity &tx);

			// Peer's data base interface
			bool putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool deletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool deleteAllPeers(const std::string &iso);
			std::vector<PeerEntity> getAllPeers(const std::string &iso) const;

		private:
			Sqlite                _sqlite;
			PeerDataSource        _peerDataSource;
			TransactionDataStore  _transactionDataStore;
			MerkleBlockDataSource _merkleBlockDataSource;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
