// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include "DatabaseManager.h"

namespace Elastos {
	namespace SDK {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_transactionDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite) {

		}

		DatabaseManager::DatabaseManager() :
			DatabaseManager("spv_wallet.db") {

		}

		DatabaseManager::~DatabaseManager() {

		}

		bool DatabaseManager::putTransaction(const std::string &iso, const TransactionEntity &tx) {
			return _transactionDataStore.putTransaction(iso, tx);
		}

		bool DatabaseManager::deleteAllTransactions(const std::string &iso) {
			return _transactionDataStore.deleteAllTransactions(iso);
		}

		std::vector<TransactionEntity> DatabaseManager::getAllTransactions(const std::string &iso) const {
			return _transactionDataStore.getAllTransactions(iso);
		}

		bool DatabaseManager::updateTransaction(const std::string &iso, const TransactionEntity &txEntity) {
			return _transactionDataStore.updateTransaction(iso, txEntity);
		}

		bool DatabaseManager::deleteTxByHash(const std::string &iso, const std::string &hash) {
			return _transactionDataStore.deleteTxByHash(iso, hash);
		}


		bool DatabaseManager::putPeer(const std::string &iso, const PeerEntity &peerEntity) {
			return _peerDataSource.putPeer(iso, peerEntity);
		}

		bool DatabaseManager::putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			return _peerDataSource.putPeers(iso, peerEntities);
		}

		bool DatabaseManager::deletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			return _peerDataSource.deletePeer(iso, peerEntity);
		}

		bool DatabaseManager::deleteAllPeers(const std::string &iso) {
			return _peerDataSource.deleteAllPeers(iso);
		}

		std::vector<PeerEntity> DatabaseManager::getAllPeers(const std::string &iso) const {
			return _peerDataSource.getAllPeers(iso);
		}


		bool DatabaseManager::putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return _merkleBlockDataSource.putMerkleBlock(iso, blockEntity);
		}

		bool DatabaseManager::putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities) {
			return _merkleBlockDataSource.putMerkleBlocks(iso, blockEntities);
		}

		bool DatabaseManager::deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return _merkleBlockDataSource.deleteMerkleBlock(iso, blockEntity);
		}

		bool DatabaseManager::deleteAllBlocks(const std::string &iso) {
			return _merkleBlockDataSource.deleteAllBlocks(iso);
		}

		std::vector<MerkleBlockEntity> DatabaseManager::getAllMerkleBlocks(const std::string &iso) const {
			return _merkleBlockDataSource.getAllMerkleBlocks(iso);
		}

		const boost::filesystem::path &DatabaseManager::getPath() const {
			return _path;
		}

	}
}
