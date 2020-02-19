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

#ifndef __ELASTOS_SDK_ASSETDATASTORE_H__
#define __ELASTOS_SDK_ASSETDATASTORE_H__

#include "TableBase.h"
#include <Common/BigInt.h>

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
			AssetDataStore(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~AssetDataStore();

			virtual void InitializeTable();

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
