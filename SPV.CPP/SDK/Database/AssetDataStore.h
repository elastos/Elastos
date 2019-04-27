// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ASSETDATASTORE_H__
#define __ELASTOS_SDK_ASSETDATASTORE_H__

#include "TableBase.h"
#include <SDK/Common/BigInt.h>

namespace Elastos {
	namespace ElaWallet {

		struct AssetEntity {
			AssetEntity() :
				Amount(0) {

			}

			AssetEntity(const std::string &assetID, const BigInt &amount, const bytes_t &Asset) :
					AssetID(assetID),
					Amount(amount),
					Asset(Asset) {
			}

			std::string AssetID;
			BigInt Amount;
			bytes_t Asset;
		};

		class AssetDataStore : public TableBase {
		public:
			AssetDataStore(Sqlite *sqlite);

			AssetDataStore(SqliteTransactionType type, Sqlite *sqlite);

			~AssetDataStore();

			bool PutAsset(const std::string &iso, const AssetEntity &asset);

			bool DeleteAsset(const std::string &assetID);

			bool DeleteAllAssets();

			bool GetAssetDetails(const std::string &assetID, AssetEntity &asset) const;

			std::vector<AssetEntity> GetAllAssets() const;

		private:
			bool SelectAsset(const std::string &assetID, AssetEntity &asset) const;

			bool InsertAsset(const std::string &iso, const AssetEntity &asset);

			bool UpdateAsset(const std::string &iso, const AssetEntity &asset);

		private:
			/*
			 * asset data table
			 */
			const std::string ASSET_OLD_TABLE_NAME = "assetTable";
			const std::string ASSET_TABLE_NAME = "registeredAssetTable";
			const std::string ASSET_COLUMN_ID = "_id";
			const std::string ASSET_AMOUNT = "assetAmount";
			const std::string ASSET_BUFF = "assetBuff";
			const std::string ASSET_ISO = "assetISO";

			const std::string ASSET_DATABASE_CREATE = "create table if not exists " + ASSET_TABLE_NAME + " (" +
				ASSET_COLUMN_ID + " text not null, " +
				ASSET_AMOUNT + " text DEFAULT '0', " +
				ASSET_BUFF + " blob, " +
				ASSET_ISO + " text DEFAULT 'ELA');";
		};

	}
}


#endif //__ELASTOS_SDK_ASSETDATASTORE_H__
