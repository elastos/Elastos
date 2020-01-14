// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "DatabaseManager.h"

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_peerBlackList(&_sqlite),
			_coinbaseDataStore(&_sqlite),
			_transactionDataStore(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite),
			_didDataStore(&_sqlite){}

		DatabaseManager::DatabaseManager() : DatabaseManager("spv_wallet.db") {}

		DatabaseManager::~DatabaseManager() {}

		bool DatabaseManager::PutCoinbases(const std::vector<TransactionPtr> &entitys) {
			return _coinbaseDataStore.PutTransactions("ela1", entitys);
		}

		bool DatabaseManager::PutCoinbase(const TransactionPtr &entity) {
			return _coinbaseDataStore.PutTransaction("ela1", entity);
		}

		bool DatabaseManager::DeleteAllCoinbase() {
			return _coinbaseDataStore.DeleteAllTransactions();
		}

		size_t DatabaseManager::GetCoinbaseTotalCount() const {
			return _coinbaseDataStore.GetAllTransactionsCount();
		}

		std::vector<TransactionPtr> DatabaseManager::GetAllCoinbase() const {
			return _coinbaseDataStore.GetAllTransactions("ELA");
		}

		bool DatabaseManager::UpdateCoinbase(const std::vector<uint256> &txHashes, uint32_t blockHeight,
											 time_t timestamp) {
			return _coinbaseDataStore.UpdateTransaction(txHashes, blockHeight, timestamp);
		}

		bool DatabaseManager::DeleteCoinbase(const uint256 &hash) {
			return _coinbaseDataStore.DeleteTxByHash(hash);
		}

		bool DatabaseManager::PutTransaction(const std::string &iso, const TransactionPtr &tx) {
			return _transactionDataStore.PutTransaction(iso, tx);
		}

		bool DatabaseManager::PutTransactions(const std::string &iso, const std::vector<TransactionPtr> &txns) {
			return _transactionDataStore.PutTransactions(iso, txns);
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

		std::vector<TransactionPtr> DatabaseManager::GetAllTransactions(const std::string &chainID) const {
			return _transactionDataStore.GetAllTransactions(chainID);
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

		bool DatabaseManager::PutMerkleBlock(const std::string &iso, const MerkleBlockPtr &blockPtr) {
			return _merkleBlockDataSource.PutMerkleBlock(iso, blockPtr);
		}

		bool DatabaseManager::PutMerkleBlocks(const std::string &iso, bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			return _merkleBlockDataSource.PutMerkleBlocks(iso, replace, blocks);
		}

		bool DatabaseManager::DeleteMerkleBlock(const std::string &iso, long id) {
			return _merkleBlockDataSource.DeleteMerkleBlock(iso, id);
		}

		bool DatabaseManager::DeleteAllBlocks(const std::string &iso) {
			return _merkleBlockDataSource.DeleteAllBlocks(iso);
		}

		std::vector<MerkleBlockPtr> DatabaseManager::GetAllMerkleBlocks(const std::string &iso,
																		const std::string &chainID) const {
			return _merkleBlockDataSource.GetAllMerkleBlocks(iso, chainID);
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

		bool DatabaseManager::PutDID(const std::string &iso, const DIDEntity &didEntity) {
			return _didDataStore.PutDID(iso, didEntity);
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

		void DatabaseManager::flush() {
			_transactionDataStore.flush();
			_coinbaseDataStore.flush();
			_merkleBlockDataSource.flush();
			_peerDataSource.flush();
			_assetDataStore.flush();
			_didDataStore.flush();
		}

	} // namespace ElaWallet
} // namespace Elastos
