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

			// Transaction's database interface
			bool putTransaction(const std::string &iso, const TransactionEntity &tx);
			bool deleteAllTransactions(const std::string &iso);
			std::vector<TransactionEntity> getAllTransactions(const std::string &iso) const;
			bool updateTransaction(const std::string &iso, const TransactionEntity &txEntity);
			bool deleteTxByHash(const std::string &iso, const std::string &hash);

			// Peer's database interface
			bool putPeer(const std::string &iso, const PeerEntity &peerEntity);
			bool putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool deletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool deleteAllPeers(const std::string &iso);
			std::vector<PeerEntity> getAllPeers(const std::string &iso) const;

			// MerkleBlock's database interface
			bool putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities);
			bool deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool deleteAllBlocks(const std::string &iso);
			std::vector<MerkleBlockEntity> getAllMerkleBlocks(const std::string &iso) const;

			const boost::filesystem::path &getPath() const;

		private:
			boost::filesystem::path _path;
			Sqlite                	_sqlite;
			PeerDataSource        	_peerDataSource;
			TransactionDataStore  	_transactionDataStore;
			MerkleBlockDataSource 	_merkleBlockDataSource;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
