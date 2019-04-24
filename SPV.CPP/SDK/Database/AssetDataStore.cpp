// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "AssetDataStore.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		AssetDataStore::AssetDataStore(Sqlite *sqlite) :
				TableBase(sqlite) {
			InitializeTable(ASSET_DATABASE_CREATE);
			InitializeTable("drop table if exists " + ASSET_OLD_TABLE_NAME + ";");
		}

		AssetDataStore::AssetDataStore(SqliteTransactionType type, Sqlite *sqlite) :
				TableBase(type, sqlite) {
			InitializeTable(ASSET_DATABASE_CREATE);
			InitializeTable("drop table if exists " + ASSET_OLD_TABLE_NAME + ";");
		}

		AssetDataStore::~AssetDataStore() {

		}

		bool AssetDataStore::PutAsset(const std::string &iso, const AssetEntity &asset) {
			return DoTransaction([&iso, &asset, this]() {
				AssetEntity existAsset;

				if (SelectAsset(iso, asset.AssetID, existAsset)) {
					UpdateAsset(iso, asset);
				} else {
					InsertAsset(iso, asset);
				}
			});
		}

		bool AssetDataStore::PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets) {
			return DoTransaction([&iso, &assets, this]() {
				AssetEntity existAsset;

				for (size_t i = 0; i < assets.size(); ++i) {
					if (SelectAsset(iso, assets[i].AssetID, existAsset)) {
						UpdateAsset(iso, assets[i]);
					} else {
						InsertAsset(iso, assets[i]);
					}
				}
			});
		}

		bool AssetDataStore::DeleteAsset(const std::string &iso, const std::string &assetID) {
			return DoTransaction([&iso, &assetID, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
				   << ASSET_TABLE_NAME
				   << " WHERE " << ASSET_COLUMN_ID << " = '" << assetID << "'"
				   << " AND " << ASSET_ISO << " = '" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool AssetDataStore::DeleteAllAssets(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
				   << ASSET_TABLE_NAME
				   << " WHERE " << ASSET_ISO << " = '" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool AssetDataStore::GetAssetDetails(const std::string &iso, const std::string &assetID, AssetEntity &asset) const {
			bool found = false;

			DoTransaction([&iso, &assetID, &asset, &found, this]() {
				found = SelectAsset(iso, assetID, asset);
			});

			return found;
		}

		std::vector<AssetEntity> AssetDataStore::GetAllAssets(const std::string &iso) const {
			std::vector<AssetEntity> assets;

			DoTransaction([&iso, &assets, this]() {
				AssetEntity asset;
				std::stringstream ss;

				ss << "SELECT "
				   << ASSET_COLUMN_ID << ", "
				   << ASSET_AMOUNT << ", "
				   << ASSET_BUFF
				   << " FROM " << ASSET_TABLE_NAME
				   << " WHERE " << ASSET_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {

					asset.AssetID = _sqlite->ColumnText(stmt, 0);
					asset.Amount = (uint64_t) _sqlite->ColumnInt64(stmt, 1);

					const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 2);
					size_t len = (size_t) _sqlite->ColumnBytes(stmt, 2);

					asset.Asset.assign(pdata, pdata + len);

					assets.push_back(asset);
				}
			});

			return assets;
		}

		bool AssetDataStore::SelectAsset(const std::string &iso, const std::string &assetID, AssetEntity &asset) const {
			bool found = false;
			std::stringstream ss;

			ss << "SELECT "
				<< ASSET_AMOUNT << ", "
				<< ASSET_BUFF
				<< " FROM " << ASSET_TABLE_NAME
				<< " WHERE " << ASSET_ISO << " = '" << iso << "' "
				<< " AND " << ASSET_COLUMN_ID << " = '" << assetID << "';";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			while (SQLITE_ROW == _sqlite->Step(stmt)) {
				found = true;

				asset.AssetID = assetID;
				asset.Amount = (uint64_t) _sqlite->ColumnInt64(stmt, 0);

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);

				asset.Asset.assign(pdata, pdata + len);
			}

			return found;
		}

		bool AssetDataStore::InsertAsset(const std::string &iso, const AssetEntity &asset) {
			std::stringstream ss;

			ss << "INSERT INTO " << ASSET_TABLE_NAME << "("
				<< ASSET_COLUMN_ID << ","
				<< ASSET_AMOUNT    << ","
				<< ASSET_BUFF      << ","
				<< ASSET_ISO
				<< ") VALUES (?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			_sqlite->BindText(stmt, 1, asset.AssetID, nullptr);
			_sqlite->BindInt64(stmt, 2, asset.Amount);
			_sqlite->BindBlob(stmt, 3, asset.Asset, nullptr);
			_sqlite->BindText(stmt, 4, iso, nullptr);

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);

			return true;
		}

		bool AssetDataStore::UpdateAsset(const std::string &iso, const AssetEntity &asset) {
			std::stringstream ss;

			ss << "UPDATE " << ASSET_TABLE_NAME << " SET "
				<< ASSET_AMOUNT << " = ?, "
				<< ASSET_BUFF   << " = ? "
				<< " WHERE " << ASSET_ISO << " = '" << iso << "'"
				<< " AND " << ASSET_COLUMN_ID << " = '" << asset.AssetID << "';";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			_sqlite->BindInt64(stmt, 1, asset.Amount);
			_sqlite->BindBlob(stmt, 2, asset.Asset, nullptr);

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);

			return true;
		}

	}
}
