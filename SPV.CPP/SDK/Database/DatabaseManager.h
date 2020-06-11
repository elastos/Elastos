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
#include "TransactionNormal.h"
#include "PeerDataSource.h"
#include "PeerBlackList.h"
#include "AssetDataStore.h"
#include "TransactionCoinbase.h"
#include "TransactionPending.h"
#include "DIDDataStore.h"
#include "UTXOStore.h"
#include "AddressUsed.h"
#include "TxHashCRC.h"
#include "TxHashDPoS.h"
#include "TxHashProposal.h"
#include "TxHashDID.h"

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

			bool ReplaceTxns(const std::vector<TransactionPtr> &txConfirmed,
							 const std::vector<TransactionPtr> &txPending,
							 const std::vector<TransactionPtr> &txCoinbase);

			bool ContainTxn(const uint256 &hash) const;

			bool UpdateTxns(const std::vector<TransactionPtr> &txns);

			// CoinBase Transaction
			bool PutCoinbaseTxns(const std::vector<TransactionPtr> &txns);

			bool PutCoinbaseTxn(const TransactionPtr &txn);

			bool DeleteAllCoinbase();

			size_t GetCoinbaseTotalCount() const;

			TransactionPtr GetCoinbaseTxn(const uint256 &hash, const std::string &chainID) const;

			std::vector<TransactionPtr> GetCoinbaseTxns(const std::string &chainID, uint32_t height) const;

			std::vector<TransactionPtr> GetCoinbaseTxns(const std::string &chainID) const;

			std::vector<TransactionPtr> GetCoinbaseUTXOTxn(const std::string &chainID) const;

			std::vector<TransactionPtr> GetCoinbaseUniqueTxns(const std::string &chainID,
															  const std::set<std::string> &hashes) const;

			std::vector<TransactionPtr>
			GetCoinbaseTxns(const std::string &chainID, size_t offset, size_t limit, bool asc = false) const;

			bool UpdateCoinbaseTxn(const std::vector<TransactionPtr> &txns);

			bool DeleteCoinbaseTxn(const uint256 &hash);

			// Normal Transaction
			bool PutNormalTxn(const TransactionPtr &tx);

			bool PutNormalTxns(const std::vector<TransactionPtr> &txns);

			bool DeleteAllNormalTxns();

			size_t GetNormalTotalCount() const;

			time_t GetNormalEarliestTxnTimestamp() const;

			TransactionPtr GetNormalTxn(const uint256 &hash, const std::string &chainID) const;

			std::vector<TransactionPtr> GetNormalTxns(const std::string &chainID, uint32_t height) const;

			std::vector<TransactionPtr> GetNormalTxns(const std::string &chainID) const;

			std::vector<TransactionPtr> GetNormalUTXOTxn(const std::string &chainID) const;

			std::vector<TransactionPtr> GetNormalUniqueTxns(const std::string &chainID,
															const std::set<std::string> &hashes) const;

			std::vector<TransactionPtr>
			GetNormalTxns(const std::string &chainID, size_t offset, size_t limit, bool asc = false) const;

			bool UpdateNormalTxn(const std::vector<TransactionPtr> &txns);

			bool DeleteNormalTxn(const uint256 &hash);

			bool DeleteNormalTxn(const std::vector<uint256> &hashes);

			// Pending Transaction
			bool PutPendingTxn(const TransactionPtr &txn);

			bool PutPendingTxns(const std::vector<TransactionPtr> &txns);

			bool DeleteAllPendingTxns();

			bool DeletePendingTxn(const uint256 &hash);

			bool DeletePendingTxns(const std::vector<uint256> &hashes);

			size_t GetPendingTxnTotalCount() const;

			TransactionPtr GetPendingTxn(const uint256 &hash, const std::string &chainID) const;

			std::vector<TransactionPtr> GetAllPendingTxns(const std::string &chainID) const;

			std::vector<TransactionPtr> GetPendingUniqueTxns(const std::string &chainID,
															 const std::set<std::string> &hashes) const;

			bool ExistPendingTxnTable() const;

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

			// TxHash CRC
			bool PutTxHashCRC(const std::vector<std::string> &txHashes, bool replace = false);

			std::vector<std::string> GetTxHashCRC() const;

			bool DeleteAllTxHashCRC();

			std::vector<TransactionPtr> GetTxCRC(const std::string &chainID) const;

			// TxHash DPoS
			bool PutTxHashDPoS(const std::vector<std::string> &txHashes, bool replace = false);

			std::vector<std::string> GetTxHashDPoS() const;

			bool DeleteAllTxHashDPoS();

			std::vector<TransactionPtr> GetTxDPoS(const std::string &chainID) const;

			// TxHash Proposal
			bool PutTxHashProposal(const std::vector<std::string> &txHashes, bool replace = false);

			std::vector<std::string> GetTxHashProposal() const;

			bool DeleteAllTxHashProposal();

			std::vector<TransactionPtr> GetTxProposal(const std::string &chainID) const;

			// TxHash DID
			bool PutTxHashDID(const std::vector<std::string> &txHashes, bool replace = false);

			std::vector<std::string> GetTxHashDID() const;

			bool DeleteAllTxHashDID();

			std::vector<TransactionPtr> GetTxDID(const std::string &chainID) const;

			// TxHash common
			bool ExistTxHashTable() const;

			// common
			const boost::filesystem::path &GetPath() const;

			void flush();

		private:
			boost::filesystem::path _path;
			Sqlite _sqlite;
			PeerDataSource _peerDataSource;
			PeerBlackList _peerBlackList;
			TransactionCoinbase _transactionCoinbase;
			TransactionNormal _transactionNormal;
			TransactionPending _transactionPending;
			MerkleBlockDataSource _merkleBlockDataSource;
			AssetDataStore _assetDataStore;
			DIDDataStore _didDataStore;
			UTXOStore _utxoStore;
			AddressUsed _addressUsed;
			TxHashCRC _txHashCRC;
			TxHashDPoS _txHashDPoS;
			TxHashProposal _txHashProposal;
			TxHashDID _txHashDID;
		};

		typedef boost::shared_ptr<DatabaseManager> DatabaseManagerPtr;
		typedef boost::weak_ptr<DatabaseManager> DatabaseManagerWeakPtr;

	}
}

#endif //__ELASTOS_SDK_DATABASEMANAGER_H__
