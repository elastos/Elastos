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

#include "DatabaseManager.h"
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_peerBlackList(&_sqlite),
			_transactionCoinbase(&_sqlite),
			_transactionNormal(&_sqlite),
			_transactionPending(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite),
			_didDataStore(&_sqlite),
			_utxoStore(&_sqlite),
			_addressUsed(&_sqlite) {
			_peerDataSource.InitializeTable();
			_peerBlackList.InitializeTable();
			_transactionCoinbase.InitializeTable();
			_transactionNormal.InitializeTable();
			_transactionPending.InitializeTable();
			_assetDataStore.InitializeTable();
			_merkleBlockDataSource.InitializeTable();
			_didDataStore.InitializeTable();
			_utxoStore.InitializeTable();
			_addressUsed.InitializeTable();
		}

		DatabaseManager::DatabaseManager() : DatabaseManager("spv_wallet.db") {}

		DatabaseManager::~DatabaseManager() {}

		bool DatabaseManager::ReplaceTxns(const std::vector<TransactionPtr> &txConfirmed,
										  const std::vector<TransactionPtr> &txPending,
										  const std::vector<TransactionPtr> &txCoinbase) {
			bool r = true;
			_transactionCoinbase.RemoveOld();

			_sqlite.BeginTransaction(IMMEDIATE);
			if (!_transactionNormal._Puts(txConfirmed, true)) {
				Log::error("replace tx confirmed");
				r = false;
			}
			if (!_transactionPending._Puts(txPending, true)) {
				Log::error("replace tx pending");
				r = false;
			}
			if (!_transactionCoinbase._Puts(txCoinbase, true)) {
				Log::error("replace tx coinbase");
				r = false;
			}
			_sqlite.EndTransaction();

			return r;
		}

		bool DatabaseManager::ContainTxn(const uint256 &hash) const {
			return _transactionNormal.ContainHash(hash) ||
				   _transactionPending.ContainHash(hash) ||
				   _transactionCoinbase.ContainHash(hash);
		}

		bool DatabaseManager::UpdateTxns(const std::vector<TransactionPtr> &txns) {
			bool r = true;
			std::vector<uint256> deletePending, deleteNormal, deleteCoinbase, hashesUpdate;
			std::vector<TransactionPtr> txNormal, txCoinbase, txPending;
			for (const TransactionPtr &tx : txns) {
				if (tx->IsUnconfirmed()) {
					if (tx->IsCoinBase()) {
						deleteCoinbase.push_back(tx->GetHash());
					} else {
						deleteNormal.push_back(tx->GetHash());
					}

					txPending.push_back(tx);
				} else {
					deletePending.push_back(tx->GetHash());
					if (tx->IsCoinBase())
						txCoinbase.push_back(tx);
					else
						txNormal.push_back(tx);
				}
			}

			_sqlite.BeginTransaction(IMMEDIATE);

			if (!deletePending.empty())
				r = _transactionPending._DeleteByHashes(deletePending) && r;
			if (!deleteNormal.empty())
				r = _transactionNormal._DeleteByHashes(deleteNormal) && r;
			if (!deleteCoinbase.empty())
				r = _transactionCoinbase._DeleteByHashes(deleteCoinbase) && r;

			for (TransactionPtr &tx : txCoinbase) {
				if (_transactionCoinbase.ContainHash(tx->GetHash())) {
					r = _transactionCoinbase._Update(tx) && r;
				} else {
					r = _transactionCoinbase._Put(tx) && r;
				}
			}
			for (TransactionPtr &tx : txNormal) {
				if (_transactionNormal.ContainHash(tx->GetHash())) {
					r = _transactionNormal._Update(tx) && r;
				} else {
					r = _transactionNormal._Put(tx) && r;
				}
			}
			for (TransactionPtr &tx : txPending) {
				if (_transactionPending.ContainHash(tx->GetHash())) {
					r = _transactionPending._Update(tx) && r;
				} else {
					r = _transactionPending._Put(tx) && r;
				}
			}

			_sqlite.EndTransaction();

			return r;
		}

		bool DatabaseManager::PutCoinbaseTxns(const std::vector<TransactionPtr> &txns) {
			return _transactionCoinbase.Puts(txns);
		}

		bool DatabaseManager::PutCoinbaseTxn(const TransactionPtr &txn) {
			return _transactionCoinbase.Put(txn);
		}

		bool DatabaseManager::DeleteAllCoinbase() {
			return _transactionCoinbase.DeleteAll();
		}

		size_t DatabaseManager::GetCoinbaseTotalCount() const {
			return _transactionCoinbase.GetAllCount();
		}

		TransactionPtr DatabaseManager::GetCoinbaseTxn(const uint256 &hash, const std::string &chainID) const {
			return _transactionCoinbase.Get(hash, chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetCoinbaseTxns(const std::string &chainID, uint32_t height) const {
			return _transactionCoinbase.GetAfter(chainID, height);
		}

		std::vector<TransactionPtr> DatabaseManager::GetCoinbaseTxns(const std::string &chainID) const {
			return _transactionCoinbase.GetAll(chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetCoinbaseUTXOTxn(const std::string &chainID) const {
			return _transactionCoinbase.GetUTXOTxn(chainID, _utxoStore.GetTableName(), _utxoStore.GetTxHashColumnName());
		}

		std::vector<TransactionPtr> DatabaseManager::GetCoinbaseTxns(const std::string &chainID, size_t offset,
																	 size_t limit, bool asc) const {
			return _transactionCoinbase.Gets(chainID, offset, limit, asc);
		}

		bool DatabaseManager::UpdateCoinbaseTxn(const std::vector<TransactionPtr> &txns) {
			return _transactionCoinbase.Update(txns);
		}

		bool DatabaseManager::DeleteCoinbaseTxn(const uint256 &hash) {
			return _transactionCoinbase.DeleteByHash(hash);
		}

		bool DatabaseManager::PutNormalTxn(const TransactionPtr &tx) {
			return _transactionNormal.Put(tx);
		}

		bool DatabaseManager::PutNormalTxns(const std::vector<TransactionPtr> &txns) {
			return _transactionNormal.Puts(txns);
		}

		bool DatabaseManager::DeleteAllNormalTxns() {
			return _transactionNormal.DeleteAll();
		}

		size_t DatabaseManager::GetNormalTotalCount() const {
			return _transactionNormal.GetAllCount();
		}

		time_t DatabaseManager::GetNormalEarliestTxnTimestamp() const {
			return _transactionNormal.GetEarliestTxnTimestamp();
		}

		TransactionPtr DatabaseManager::GetNormalTxn(const uint256 &hash, const std::string &chainID) const {
			return _transactionNormal.Get(hash, chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetNormalTxns(const std::string &chainID, uint32_t height) const {
			return _transactionNormal.GetAfter(chainID, height);
		}

		std::vector<TransactionPtr> DatabaseManager::GetNormalTxns(const std::string &chainID) const {
			return _transactionNormal.GetAll(chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetNormalUTXOTxn(const std::string &chainID) const {
			return _transactionNormal.GetUTXOTxn(chainID, _utxoStore.GetTableName(), _utxoStore.GetTxHashColumnName());
		}

		std::vector<TransactionPtr> DatabaseManager::GetNormalTxns(const std::string &chainID, size_t offset,
																   size_t limit, bool asc) const {
			return _transactionNormal.Gets(chainID, offset, limit, asc);
		}

		bool DatabaseManager::UpdateNormalTxn(const std::vector<TransactionPtr> &txns) {
			return _transactionNormal.Update(txns);
		}

		bool DatabaseManager::DeleteNormalTxn(const uint256 &hash) {
			return _transactionNormal.DeleteByHash(hash);
		}

		bool DatabaseManager::DeleteNormalTxn(const std::vector<uint256> &hashes) {
			return _transactionNormal.DeleteByHashes(hashes);
		}

		bool DatabaseManager::PutPendingTxn(const TransactionPtr &txn) {
			return _transactionPending.Put(txn);
		}

		bool DatabaseManager::PutPendingTxns(const std::vector<TransactionPtr> &txns) {
			return _transactionPending.Puts(txns);
		}

		bool DatabaseManager::DeleteAllPendingTxns() {
			return _transactionPending.DeleteAll();
		}

		bool DatabaseManager::DeletePendingTxn(const uint256 &hash) {
			return _transactionPending.DeleteByHash(hash);
		}

		bool DatabaseManager::DeletePendingTxns(const std::vector<uint256> &hashes) {
			return _transactionPending.DeleteByHashes(hashes);
		}

		size_t DatabaseManager::GetPendingTxnTotalCount() const {
			return _transactionPending.GetAllCount();
		}

		TransactionPtr DatabaseManager::GetPendingTxn(const uint256 &hash, const std::string &chainID) const {
			return _transactionPending.Get(hash, chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllPendingTxns(const std::string &chainID) const {
			return _transactionPending.GetAll(chainID);
		}

		bool DatabaseManager::ExistPendingTxnTable() const {
			return _transactionPending.TableExist();
		}

		bool DatabaseManager::PutPeer(const PeerEntity &peerEntity) {
			return _peerDataSource.PutPeer(peerEntity);
		}

		bool DatabaseManager::PutPeers(const std::vector<PeerEntity> &peerEntities) {
			return _peerDataSource.PutPeers(peerEntities);
		}

		bool DatabaseManager::DeletePeer(const PeerEntity &peerEntity) {
			return _peerDataSource.DeletePeer(peerEntity);
		}

		bool DatabaseManager::DeleteAllPeers() {
			return _peerDataSource.DeleteAllPeers();
		}

		std::vector<PeerEntity> DatabaseManager::GetAllPeers() const {
			return _peerDataSource.GetAllPeers();
		}

		bool DatabaseManager::DeleteBlackPeer(const PeerEntity &entity) {
			return _peerBlackList.DeletePeer(entity);
		}

		bool DatabaseManager::DeleteAllBlackPeers() {
			return _peerBlackList.DeleteAllPeers();
		}

		std::vector<PeerEntity> DatabaseManager::GetAllBlackPeers() const {
			return _peerBlackList.GetAllPeers();
		}

		bool DatabaseManager::PutBlackPeer(const PeerEntity &entity) {
			return _peerBlackList.PutPeer(entity);
		}

		bool DatabaseManager::PutBlackPeers(const std::vector<PeerEntity> &entitys) {
			return _peerBlackList.PutPeers(entitys);
		}

		size_t DatabaseManager::GetAllPeersCount() const {
			return _peerDataSource.GetAllPeersCount();
		}

		bool DatabaseManager::PutMerkleBlock(const MerkleBlockPtr &blockPtr) {
			return _merkleBlockDataSource.PutMerkleBlock(blockPtr);
		}

		bool DatabaseManager::PutMerkleBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			return _merkleBlockDataSource.PutMerkleBlocks(replace, blocks);
		}

		bool DatabaseManager::DeleteMerkleBlock(long id) {
			return _merkleBlockDataSource.DeleteMerkleBlock(id);
		}

		bool DatabaseManager::DeleteAllBlocks() {
			return _merkleBlockDataSource.DeleteAllBlocks();
		}

		std::vector<MerkleBlockPtr> DatabaseManager::GetAllMerkleBlocks(const std::string &chainID) const {
			return _merkleBlockDataSource.GetAllMerkleBlocks(chainID);
		}

		const boost::filesystem::path &DatabaseManager::GetPath() const {
			return _path;
		}

		bool DatabaseManager::PutAsset(const std::string &iso, const AssetEntity &asset) {
			return _assetDataStore.PutAsset(iso, asset);
		}

		bool DatabaseManager::DeleteAsset(const std::string &assetID) {
			return _assetDataStore.DeleteAsset(assetID);
		}

		bool DatabaseManager::DeleteAllAssets() {
			return _assetDataStore.DeleteAllAssets();
		}

		bool DatabaseManager::GetAssetDetails(const std::string &assetID, AssetEntity &asset) const {
			return _assetDataStore.GetAssetDetails(assetID, asset);
		}

		std::vector<AssetEntity> DatabaseManager::GetAllAssets() const {
			return _assetDataStore.GetAllAssets();
		}

		bool DatabaseManager::PutUTXOs(const std::vector<UTXOEntity> &entities) {
			return _utxoStore.Puts(entities);
		}

		std::vector<UTXOEntity> DatabaseManager::GetUTXOs() const {
			return _utxoStore.Gets();
		}

		bool DatabaseManager::Update(const std::vector<UTXOEntity> &added, const std::vector<UTXOEntity> &deleted,
									 bool replace) {
			return _utxoStore.Update(added, deleted, replace);
		}

		bool DatabaseManager::DeleteAllUTXOs() {
			return _utxoStore.DeleteAll();
		}

		bool DatabaseManager::DeleteUTXOs(const std::vector<UTXOEntity> &entities) {
			return _utxoStore.Delete(entities);
		}

		bool DatabaseManager::PutUsedAddresses(const std::vector<std::string> &addresses, bool replace) {
			return _addressUsed.Puts(addresses, replace);
		}

		std::vector<std::string> DatabaseManager::GetUsedAddresses() const {
			return _addressUsed.Gets();
		}

		bool DatabaseManager::DeleteAllUsedAddresses() {
			return _addressUsed.DeleteAll();
		}

		void DatabaseManager::flush() {
			_transactionNormal.flush();
			_transactionCoinbase.flush();
			_transactionPending.flush();
			_merkleBlockDataSource.flush();
			_peerDataSource.flush();
			_assetDataStore.flush();
			_didDataStore.flush();
			_utxoStore.flush();
			_addressUsed.flush();
		}

	} // namespace ElaWallet
} // namespace Elastos
