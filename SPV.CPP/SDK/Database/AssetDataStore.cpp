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

#include "AssetDataStore.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		AssetDataStore::AssetDataStore(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction(ASSET_DATABASE_CREATE);
			TableBase::ExecInTransaction("DROP TABLE IF EXISTS " + ASSET_OLD_TABLE_NAME + ";");
		}

		AssetDataStore::~AssetDataStore() {
		}

		void AssetDataStore::InitializeTable() {
			TableBase::ExecInTransaction(ASSET_DATABASE_CREATE);
			TableBase::ExecInTransaction("DROP TABLE IF EXISTS " + ASSET_OLD_TABLE_NAME + ";");
		}

		bool AssetDataStore::PutAsset(const std::string &iso, const AssetEntity &asset) {
			AssetEntity existAsset;
			if (SelectAsset(asset.AssetID, existAsset)) {
				return DoTransaction([&iso, &asset, this]() {
					return UpdateAsset(iso, asset);
				});
			}

			return DoTransaction([&iso, &asset, this]() {
				return InsertAsset(iso, asset);
			});
		}

		bool AssetDataStore::DeleteAsset(const std::string &assetID) {
			return DoTransaction([&assetID, this]() {
				std::string sql = "DELETE FROM " + ASSET_TABLE_NAME + " WHERE " + ASSET_COLUMN_ID + " = ?;";

				return SqliteWrapper(sql, [&assetID, this](sqlite3_stmt *stmt) {
					if (!_sqlite->BindText(stmt, 1, assetID, nullptr)) {
						Log::error("bind text");
						return false;
					}

					if (SQLITE_DONE != _sqlite->Step(stmt)) {
						Log::error("step");
						return false;
					}

					return true;
				});
			});
		}

		bool AssetDataStore::DeleteAllAssets() {
			return DeleteAll(ASSET_TABLE_NAME);
		}

		bool AssetDataStore::GetAssetDetails(const std::string &assetID, AssetEntity &asset) const {
			return SelectAsset(assetID, asset);
		}

		std::vector<AssetEntity> AssetDataStore::GetAllAssets() const {
			std::vector<AssetEntity> assets;

			std::string sql = "SELECT " + ASSET_COLUMN_ID + ", " + ASSET_AMOUNT + ", " + ASSET_BUFF + " FROM " +
				  ASSET_TABLE_NAME + ";";

			if (!SqliteWrapper(sql, [&assets, this](sqlite3_stmt *stmt) {
				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					AssetEntity asset;
					asset.AssetID = _sqlite->ColumnText(stmt, 0);
					asset.Amount.setDec(_sqlite->ColumnText(stmt, 1));
					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 2);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 2);
					asset.Asset.assign(pdata, pdata + len);
					assets.push_back(asset);
				}

				return true;
			}))  {
				return {};
			}

			return assets;
		}

		bool AssetDataStore::SelectAsset(const std::string &assetID, AssetEntity &asset) const {
			std::string sql = "SELECT " + ASSET_AMOUNT + ", " + ASSET_BUFF + " FROM " + ASSET_TABLE_NAME + " WHERE " +
				  ASSET_COLUMN_ID + " = ?;";

			return SqliteWrapper(sql, [&assetID, &asset, this](sqlite3_stmt *stmt) {
				bool found = false;

				if (!_sqlite->BindText(stmt, 1, assetID, nullptr)) {
					Log::error("bind text");
					return false;
				}

				if (SQLITE_ROW == _sqlite->Step(stmt)) {
					found = true;

					asset.AssetID = assetID;
					asset.Amount.setDec(_sqlite->ColumnText(stmt, 0));

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);

					asset.Asset.assign(pdata, pdata + len);
				}

				return found;
			});
		}

		bool AssetDataStore::InsertAsset(const std::string &iso, const AssetEntity &asset) {
			std::string sql = "INSERT INTO " + ASSET_TABLE_NAME + "(" + ASSET_COLUMN_ID + "," +
				  ASSET_AMOUNT + "," + ASSET_BUFF + "," + ASSET_ISO + ") VALUES (?, ?, ?, ?);";

			return SqliteWrapper(sql, [&iso, &asset, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, asset.AssetID, nullptr) ||
					!_sqlite->BindText(stmt, 2, asset.Amount.getDec(), nullptr) ||
					!_sqlite->BindBlob(stmt, 3, asset.Asset, nullptr) ||
					!_sqlite->BindText(stmt, 4, iso, nullptr)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				return true;
			});
		}

		bool AssetDataStore::UpdateAsset(const std::string &iso, const AssetEntity &asset) {
			std::string sql = "UPDATE " + ASSET_TABLE_NAME + " SET "
							  + ASSET_AMOUNT + " = ?, "
							  + ASSET_BUFF + " = ?, "
							  + ASSET_ISO + " = ?"
							  + " WHERE " + ASSET_COLUMN_ID + " = ?;";

			return SqliteWrapper(sql, [&iso, &asset, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindText(stmt, 1, asset.Amount.getDec(), nullptr) ||
					!_sqlite->BindBlob(stmt, 2, asset.Asset, nullptr) ||
					!_sqlite->BindText(stmt, 3, iso, nullptr) ||
					!_sqlite->BindText(stmt, 4, asset.AssetID, nullptr)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				return true;
			});
		}

	}
}
