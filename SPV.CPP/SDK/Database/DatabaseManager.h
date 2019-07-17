// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "MerkleBlockDataSource.h"
#include "TransactionDataStore.h"
#include "PeerDataSource.h"
#include "AssetDataStore.h"
#include "CoinBaseUTXODataStore.h"
#include "Sqlite.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXO;
		typedef boost::shared_ptr<UTXO> UTXOPtr;

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);
			DatabaseManager();
			~DatabaseManager();

			// CoinBase UTXO database interface
			bool PutCoinBase(const std::vector<UTXOPtr> &entitys);
			bool PutCoinBase(const UTXOPtr &entity);
			bool DeleteAllCoinBase();
			size_t GetCoinBaseTotalCount() const;
			std::vector<UTXOPtr> GetAllCoinBase() const;
			bool UpdateCoinBase(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);
			bool UpdateSpentCoinBase(const std::vector<uint256> &txHashes);
			bool DeleteCoinBase(const uint256 &hash);

			// Transaction's database interface
			bool PutTransaction(const std::string &iso, const TransactionEntity &tx);
			bool DeleteAllTransactions(const std::string &iso);
			size_t GetAllTransactionsCount(const std::string &iso) const;
			std::vector<TransactionEntity> GetAllTransactions(const std::string &iso) const;
			bool UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);
			bool DeleteTxByHash(const std::string &iso, const std::string &hash);
			bool DeleteTxByHashes(const std::vector<std::string> &hashes);

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
			bool DeleteAsset(const std::string &assetID);
			bool DeleteAllAssets();
			bool GetAssetDetails(const std::string &assetID, AssetEntity &asset) const;
			std::vector<AssetEntity> GetAllAssets() const;

			const boost::filesystem::path &GetPath() const;

		private:
			boost::filesystem::path _path;
			Sqlite                	_sqlite;
			PeerDataSource        	_peerDataSource;
			CoinBaseUTXODataStore   _coinbaseDataStore;
			TransactionDataStore  	_transactionDataStore;
			MerkleBlockDataSource 	_merkleBlockDataSource;
			AssetDataStore          _assetDataStore;
		};

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
