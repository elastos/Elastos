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

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_peerBlackList(&_sqlite),
			_transactionCoinbase(&_sqlite),
			_transactionDataStore(&_sqlite),
			_transactionPending(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite),
			_didDataStore(&_sqlite),
			_utxoStore(&_sqlite) {
			_peerDataSource.InitializeTable();
			_peerBlackList.InitializeTable();
			_transactionCoinbase.InitializeTable();
			_transactionDataStore.InitializeTable();
			//_transactionPending.InitializeTable();
			_assetDataStore.InitializeTable();
			_merkleBlockDataSource.InitializeTable();
			_didDataStore.InitializeTable();
			_utxoStore.InitializeTable();
		}

		DatabaseManager::DatabaseManager() : DatabaseManager("spv_wallet.db") {}

		DatabaseManager::~DatabaseManager() {}

		bool DatabaseManager::PutCoinbases(const std::vector<TransactionPtr> &entitys) {
			return _transactionCoinbase.PutTransactions(entitys);
		}

		bool DatabaseManager::PutCoinbase(const TransactionPtr &entity) {
			return _transactionCoinbase.PutTransaction(entity);
		}

		bool DatabaseManager::DeleteAllCoinbase() {
			return _transactionCoinbase.DeleteAllTransactions();
		}

		size_t DatabaseManager::GetCoinbaseTotalCount() const {
			return _transactionCoinbase.GetAllTransactionsCount();
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllCoinbase(const std::string &chainID) const {
			return _transactionCoinbase.GetAllConfirmedTxns(chainID);
		}

		bool DatabaseManager::UpdateCoinbase(const std::vector<uint256> &txHashes, uint32_t blockHeight,
											 time_t timestamp) {
			return _transactionCoinbase.UpdateTransaction(txHashes, blockHeight, timestamp);
		}

		bool DatabaseManager::DeleteCoinbase(const uint256 &hash) {
			return _transactionCoinbase.DeleteTxByHash(hash);
		}

		bool DatabaseManager::PutTransaction(const TransactionPtr &tx) {
			return _transactionDataStore.PutTransaction(tx);
		}

		bool DatabaseManager::PutTransactions(const std::vector<TransactionPtr> &txns) {
			return _transactionDataStore.PutTransactions(txns);
		}

		bool DatabaseManager::DeleteAllTransactions() {
			return _transactionDataStore.DeleteAllTransactions();
		}

		size_t DatabaseManager::GetAllTransactionsCount() const {
			return _transactionDataStore.GetAllTransactionsCount();
		}

		TransactionPtr DatabaseManager::GetTransaction(const uint256 &hash, const std::string &chainID) {
			return _transactionDataStore.GetTransaction(hash, chainID);
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllConfirmedTxns(const std::string &chainID) const {
			return _transactionDataStore.GetAllConfirmedTxns(chainID);
		}

		bool DatabaseManager::UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight,
												time_t timestamp) {
			return _transactionDataStore.UpdateTransaction(hashes, blockHeight, timestamp);
		}

		bool DatabaseManager::DeleteTxByHash(const uint256 &hash) {
			return _transactionDataStore.DeleteTxByHash(hash);
		}

		bool DatabaseManager::DeleteTxByHashes(const std::vector<uint256> &hashes) {
			return _transactionDataStore.DeleteTxByHashes(hashes);
		}

		bool DatabaseManager::PutPendingTxn(const TransactionPtr &txn) {
			return _transactionPending.PutTransaction(txn);
		}

		bool DatabaseManager::PutPendingTxns(const std::vector<TransactionPtr> txns) {
			return _transactionPending.PutTransactions(txns);
		}

		bool DatabaseManager::DeleteAllPendingTxns() {
			return _transactionPending.DeleteAllTransactions();
		}

		bool DatabaseManager::DeletePendingTxn(const uint256 &hash) {
			return _transactionPending.DeleteTxByHash(hash);
		}

		size_t DatabaseManager::GetPendingTxnTotalCount() const {
			return _transactionPending.GetAllTransactionsCount();
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllPendingTxns(const std::string &chainID) const {
			return _transactionPending.GetAllConfirmedTxns(chainID);
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

		bool DatabaseManager::PutDID(const DIDEntity &didEntity) {
			return _didDataStore.PutDID(didEntity);
		}

		bool DatabaseManager::UpdateDID(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			return _didDataStore.UpdateDID(hashes, blockHeight, timestamp);
		}

		bool DatabaseManager::DeleteDID(const std::string &did) {
			return _didDataStore.DeleteDID(did);
		}

		bool DatabaseManager::DeleteDIDByTxHash(const std::string &txHash) {
			return _didDataStore.DeleteDIDByTxHash(txHash);
		}

		std::string DatabaseManager::GetDIDByTxHash(const std::string &txHash) const {
			return _didDataStore.GetDIDByTxHash(txHash);
		}

		bool DatabaseManager::GetDIDDetails(const std::string &did, DIDEntity &didEntity) const {
			return _didDataStore.GetDIDDetails(did, didEntity);
		}

		std::vector<DIDEntity> DatabaseManager::GetAllDID() const {
			return _didDataStore.GetAllDID();
		}

		bool DatabaseManager::DeleteAllDID() {
			return _didDataStore.DeleteAllDID();
		}

		bool DatabaseManager::PutUTXOs(const std::vector<UTXOEntity> &entities) {
			return _utxoStore.Puts(entities);
		}

		std::vector<UTXOEntity> DatabaseManager::GetUTXOs() const {
			return _utxoStore.Gets();
		}

		bool DatabaseManager::DeleteAllUTXOs() {
			return _utxoStore.DeleteAll();
		}

		bool DatabaseManager::DeleteUTXOs(const std::vector<UTXOEntity> &entities) {
			return _utxoStore.Delete(entities);
		}

		bool DatabaseManager::ExistUTXOTable() const {
			return _utxoStore.TableExist();
		}

		void DatabaseManager::flush() {
			_transactionDataStore.flush();
			_transactionCoinbase.flush();
			_merkleBlockDataSource.flush();
			_peerDataSource.flush();
			_assetDataStore.flush();
			_didDataStore.flush();
			_utxoStore.flush();
		}

	} // namespace ElaWallet
} // namespace Elastos
