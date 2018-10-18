// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AssetDataStore.h"

namespace Elastos {
	namespace ElaWallet {

		AssetDataStore::AssetDataStore(Sqlite *sqlite) :
				TableBase(sqlite) {

		}

		AssetDataStore::AssetDataStore(SqliteTransactionType type, Sqlite *sqlite) :
				TableBase(type, sqlite) {

		}

		AssetDataStore::~AssetDataStore() {

		}

		bool AssetDataStore::PutAsset(const std::string &iso, const AssetEntity &asset) {
			//todo complete me
			return false;
		}

		bool AssetDataStore::PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets) {
			//todo complete me
			return false;
		}

		bool AssetDataStore::DeleteAsset(const std::string &iso, const UInt256 &assetID) {
			//todo complete me
			return false;
		}

		bool AssetDataStore::DeleteAllAssets(const std::string &iso) {
			//todo complete me
			return false;
		}

		AssetEntity AssetDataStore::GetAssetDetails(uint32_t assetTableID) {
			//todo complete me
			return AssetEntity(Asset(), 0, UINT256_ZERO);
		}

		std::vector<AssetEntity> AssetDataStore::GetAllMerkleBlocks(const std::string &iso) const {
			//todo complete me
			return std::vector<AssetEntity>();
		}

	}
}
