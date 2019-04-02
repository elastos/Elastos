// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "MerkleBlockDataSource.h"
#include "TransactionDataStore.h"
#include "PeerDataSource.h"
#include "AssetDataStore.h"
#include "Sqlite.h"

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);
			DatabaseManager();
			~DatabaseManager();

			// Transaction's database interface
			bool PutTransaction(const std::string &iso, const TransactionEntity &tx);
			bool DeleteAllTransactions(const std::string &iso);
			size_t GetAllTransactionsCount(const std::string &iso) const;
			std::vector<TransactionEntity> GetAllTransactions(const std::string &iso) const;
			bool UpdateTransaction(const std::string &iso, const TransactionEntity &txEntity);
			bool DeleteTxByHash(const std::string &iso, const std::string &hash);

			// Peer's database interface
			bool PutPeer(const std::string &iso, const PeerEntity &peerEntity);
			bool PutPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool DeletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool DeleteAllPeers(const std::string &iso);
			size_t GetAllPeersCount(const std::string &iso) const;
			std::vector<PeerEntity> GetAllPeers(const std::string &iso) const;

			// MerkleBlock's database interface
			bool PutMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool PutMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities);
			bool DeleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool DeleteAllBlocks(const std::string &iso);
			std::vector<MerkleBlockEntity> GetAllMerkleBlocks(const std::string &iso) const;

			// Asset's database interface
			bool PutAsset(const std::string &iso, const AssetEntity &asset);
			bool PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets);
			bool DeleteAsset(const std::string &iso, const std::string &assetID);
			bool DeleteAllAssets(const std::string &iso);
			bool GetAssetDetails(const std::string &iso, const std::string &assetID, AssetEntity &asset) const;
			std::vector<AssetEntity> GetAllAssets(const std::string &iso) const;

			const boost::filesystem::path &GetPath() const;

		private:
			boost::filesystem::path _path;
			Sqlite                	_sqlite;
			PeerDataSource        	_peerDataSource;
			TransactionDataStore  	_transactionDataStore;
			MerkleBlockDataSource 	_merkleBlockDataSource;
			AssetDataStore 			_assetDataStore;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
