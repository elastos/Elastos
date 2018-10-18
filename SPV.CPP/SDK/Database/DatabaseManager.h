// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "MerkleBlockDataSource.h"
#include "TransactionDataStore.h"
#include "PeerDataSource.h"
#include "AssetDataStore.h"
#include "ExternalAddresses.h"
#include "InternalAddresses.h"
#include "Sqlite.h"

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);
			DatabaseManager();
			~DatabaseManager();

			// Transaction's database interface
			bool putTransaction(const std::string &iso, const TransactionEntity &tx);
			bool deleteAllTransactions(const std::string &iso);
			size_t getAllTransactionsCount(const std::string &iso) const;
			std::vector<TransactionEntity> getAllTransactions(const std::string &iso) const;
			bool updateTransaction(const std::string &iso, const TransactionEntity &txEntity);
			bool deleteTxByHash(const std::string &iso, const std::string &hash);

			// Peer's database interface
			bool putPeer(const std::string &iso, const PeerEntity &peerEntity);
			bool putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool deletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool deleteAllPeers(const std::string &iso);
			size_t getAllPeersCount(const std::string &iso) const;
			std::vector<PeerEntity> getAllPeers(const std::string &iso) const;

			// MerkleBlock's database interface
			bool putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities);
			bool deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool deleteAllBlocks(const std::string &iso);
			std::vector<MerkleBlockEntity> getAllMerkleBlocks(const std::string &iso) const;

			// Asset's database interface
			bool PutAsset(const std::string &iso, const AssetEntity &asset);
			bool PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets);
			bool DeleteAsset(const std::string &iso, const UInt256 &assetID);
			bool DeleteAllAssets(const std::string &iso);
			AssetEntity GetAssetDetails(uint32_t assetTableID);
			std::vector<AssetEntity> GetAllMerkleBlocks(const std::string &iso) const;


			// InternalAddresses's database interface
			bool putInternalAddress(uint32_t startIndex, const std::string &address);
			bool putInternalAddresses(uint32_t startIndex, const std::vector<std::string> &addresses);
			bool clearInternalAddresses();
			std::vector<std::string> getInternalAddresses(uint32_t startIndex, uint32_t count);
			uint32_t getInternalAvailableAddresses(uint32_t startIndex);

			// ExternalAddresses's database interface
			bool putExternalAddress(uint32_t startIndex, const std::string &address);
			bool putExternalAddresses(uint32_t startIndex, const std::vector<std::string> &addresses);
			bool clearExternalAddresses();
			std::vector<std::string> getExternalAddresses(uint32_t startIndex, uint32_t count);
			uint32_t getExternalAvailableAddresses(uint32_t startIndex);

			const boost::filesystem::path &getPath() const;

		private:
			boost::filesystem::path _path;
			Sqlite                	_sqlite;
			ExternalAddresses		_externalAddresses;
			InternalAddresses		_internalAddresses;
			PeerDataSource        	_peerDataSource;
			TransactionDataStore  	_transactionDataStore;
			MerkleBlockDataSource 	_merkleBlockDataSource;
			AssetDataStore 			_assetDataStore;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
