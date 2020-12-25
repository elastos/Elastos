/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ELASTOS_SDK_DATABASEMANAGER_H__
#define __ELASTOS_SDK_DATABASEMANAGER_H__

#include "MerkleBlockDataSource.h"
#include "PeerDataSource.h"
#include "PeerBlackList.h"
#include "AssetDataStore.h"
#include "UTXOStore.h"
#include "AddressUsed.h"
#include "TxTable.h"
#include "DataMigrate.h"

namespace Elastos {
	namespace ElaWallet {

		class UTXO;

		typedef boost::shared_ptr<UTXO> UTXOPtr;

		class DatabaseManager {
		public:
			DatabaseManager(const boost::filesystem::path &path);

			DatabaseManager();

			~DatabaseManager();

			void ClearData();

			bool TxTableDataMigrateDone();

			bool SetTxTableDataMigrateDone();

			bool ContainTx(const std::string &hash) const;

			bool GetUTXOTx(std::vector<TxEntity> &entities) const;

			bool GetTx(std::vector<TxEntity> &entities, uint32_t height) const;

			bool GetTx(std::vector<TxEntity> &entities, const std::vector<uint8_t> &types) const;

			bool GetTx(std::vector<TxEntity> &entities) const;

			bool GetTx(std::vector<TxEntity> &entities, const std::set<std::string> &hashes) const;

			bool GetTx(std::vector<TxEntity> &entities, uint8_t type, bool invertMatch, size_t offset, size_t limit, bool desc) const;

			size_t GetTxCnt(uint8_t type, bool invertMatch) const;

			size_t GetAllTxCnt() const;

			time_t GetEarliestTxTimestamp() const;

			bool PutTx(const std::vector<TxEntity> &entities);

			bool UpdateTx(const std::vector<std::string> &hashes, uint32_t height, time_t timestamp);

			bool DeleteTx(const std::string &hash);

			bool DeleteAllTx();

			bool GetAllOldTx(std::vector<TxOldEntity> &entities) const;

			// Peer Address
			bool PutPeer(const PeerEntity &peerEntity);

			bool PutPeers(const std::vector<PeerEntity> &peerEntities);

			bool DeletePeer(const PeerEntity &peerEntity);

			bool DeleteAllPeers();

			size_t GetAllPeersCount() const;

			std::vector<PeerEntity> GetAllPeers() const;

			// Peer Blacklist
			bool PutBlackPeer(const PeerEntity &entity);

			bool PutBlackPeers(const std::vector<PeerEntity> &entitys);

			bool DeleteBlackPeer(const PeerEntity &entity);

			bool DeleteAllBlackPeers();

			std::vector<PeerEntity> GetAllBlackPeers() const;

			// MerkleBlock
			bool PutMerkleBlock(const MerkleBlockPtr &blockPtr);

			bool PutMerkleBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			bool DeleteMerkleBlock(long id);

			bool DeleteAllBlocks();

			std::vector<MerkleBlockPtr> GetAllMerkleBlocks(const std::string &chainID) const;

			// Asset
			bool PutAsset(const std::string &iso, const AssetEntity &asset);

			bool DeleteAsset(const std::string &assetID);

			bool DeleteAllAssets();

			bool GetAssetDetails(const std::string &assetID, AssetEntity &asset) const;

			std::vector<AssetEntity> GetAllAssets() const;

			// UTXO store
			bool PutUTXOs(const std::vector<UTXOEntity> &entities);

			std::vector<UTXOEntity> GetUTXOs() const;

			bool UTXOUpdate(const std::vector<UTXOEntity> &added, const std::vector<UTXOEntity> &deleted, bool replace);

			bool DeleteAllUTXOs();

			bool DeleteUTXOs(const std::vector<UTXOEntity> &entities);

			// Used Address
			bool PutUsedAddresses(const std::vector<std::string> &addresses, bool replace);

			std::vector<std::string> GetUsedAddresses() const;

			bool DeleteAllUsedAddresses();

			bool TxTableDataMigrateDone() const;

			// common
			const boost::filesystem::path &GetPath() const;

			void flush();

		private:
			boost::filesystem::path _path;
			Sqlite _sqlite;
			PeerDataSource _peerDataSource;
			PeerBlackList _peerBlackList;
			MerkleBlockDataSource _merkleBlockDataSource;
			AssetDataStore _assetDataStore;
			UTXOStore _utxoStore;
			AddressUsed _addressUsed;
			TxTable _txTable;
			DataMigrate _dataMigrate;
		};

		typedef boost::shared_ptr<DatabaseManager> DatabaseManagerPtr;
		typedef boost::weak_ptr<DatabaseManager> DatabaseManagerWeakPtr;

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
