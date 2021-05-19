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
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		DatabaseManager::DatabaseManager(const boost::filesystem::path &path) :
			_path(path),
			_sqlite(path),
			_peerDataSource(&_sqlite),
			_peerBlackList(&_sqlite),
			_assetDataStore(&_sqlite),
			_merkleBlockDataSource(&_sqlite),
			_utxoStore(&_sqlite),
			_addressUsed(&_sqlite),
			_txTable(&_sqlite),
			_settings(&_sqlite)
		{
		}

		DatabaseManager::DatabaseManager() : DatabaseManager("spv_wallet.db") {}

		DatabaseManager::~DatabaseManager() {}

		void DatabaseManager::ClearData() {
			_merkleBlockDataSource.DeleteAllBlocks();
			_utxoStore.DeleteAll();
			_addressUsed.DeleteAll();
		}

		bool DatabaseManager::TxTableDataMigrateDone() const {
			return _settings.GetSetting(_txTable.GetTableName() + "Migrate") == 1;
		}

		bool DatabaseManager::SetTxTableDataMigrateDone() {
			bool r = false;

			_sqlite.BeginTransaction(IMMEDIATE);

			if (_settings.PutSettingInner(_txTable.GetTableName() + "Migrate", 1)) {
				if (_txTable.RemoveOldTxTableInner()) {
					r = true;
				} else {
					Log::error("remove old tx table");
				}
			} else {
				Log::error("set data migrate done");
			}

			_sqlite.EndTransaction();

			return r;
		}

		int DatabaseManager::GetSyncMode() const {
			return _settings.GetSetting("syncMode");
		}

		bool DatabaseManager::SetSyncMode(int mode) {
			return _settings.PutSetting("syncMode", mode);
		}

		bool DatabaseManager::ContainTx(const std::string &hash) const {
			return _txTable.ContainTx(hash);
		}

		bool DatabaseManager::GetUTXOTx(std::vector<TxEntity> &entities) const {
			return _txTable.GetTx(entities, _utxoStore.GetTableName(), _utxoStore.GetTxHashColumnName());
		}

		bool DatabaseManager::GetTx(std::vector<TxEntity> &entities, uint32_t height) const {
			return _txTable.GetTx(entities, height);
		}

		bool DatabaseManager::GetTx(std::vector<TxEntity> &entities, const std::vector<uint8_t> &types) const {
			return _txTable.GetTx(entities, types);
		}

		bool DatabaseManager::GetTx(std::vector<TxEntity> &entities) const {
			return _txTable.GetTx(entities);
		}

		bool DatabaseManager::GetTx(std::vector<TxEntity> &entities, const std::set<std::string> &hashes) const {
			return _txTable.GetTx(entities, hashes);
		}

		bool DatabaseManager::GetTx(std::vector<TxEntity> &entities, uint8_t type, bool invertMatch, size_t offset, size_t limit, bool desc) const {
			return _txTable.GetTx(entities, type, invertMatch, offset, limit, desc);
		}

		size_t DatabaseManager::GetTxCnt(uint8_t type, bool invertMatch) const {
			return _txTable.GetTxCnt(type, invertMatch);
		}

		size_t DatabaseManager::GetAllTxCnt() const {
			return _txTable.GetAllTxCnt();
		}

		time_t DatabaseManager::GetEarliestTxTimestamp() const {
			return _txTable.GetEarliestTxTimestamp();
		}

		bool DatabaseManager::PutTx(const std::vector<TxEntity> &entities) {
			return _txTable.PutTx(entities);
		}

		bool DatabaseManager::UpdateTx(const std::vector<std::string> &hashes, uint32_t height, time_t timestamp) {
			return _txTable.UpdateTx(hashes, height, timestamp);
		}

		bool DatabaseManager::DeleteTx(const std::string &hash) {
			return _txTable.DeleteTx(hash);
		}

		bool DatabaseManager::DeleteAllTx() {
			return _txTable.DeleteAll();
		}

		bool DatabaseManager::GetAllOldTx(std::vector<TxOldEntity> &entities) const {
			return _txTable.GetAllOldTx(entities);
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

		bool DatabaseManager::UTXOUpdate(const std::vector<UTXOEntity> &added, const std::vector<UTXOEntity> &deleted,
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
			_peerDataSource.flush();
			_peerBlackList.flush();
			_merkleBlockDataSource.flush();
			_assetDataStore.flush();
			_utxoStore.flush();
			_addressUsed.flush();
			_txTable.flush();
			_settings.flush();
		}

	} // namespace ElaWallet
} // namespace Elastos
