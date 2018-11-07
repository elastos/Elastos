// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ParamChecker.h>
#include <Core/BRInt.h>
#include <SDK/Common/Utils.h>
#include "AssetDataStore.h"

namespace Elastos {
	namespace ElaWallet {

		AssetDataStore::AssetDataStore(Sqlite *sqlite) :
				TableBase(sqlite) {
			initializeTable(ASSET_DATABASE_CREATE);
		}

		AssetDataStore::AssetDataStore(SqliteTransactionType type, Sqlite *sqlite) :
				TableBase(type, sqlite) {
			initializeTable(ASSET_DATABASE_CREATE);
		}

		AssetDataStore::~AssetDataStore() {

		}

		bool AssetDataStore::PutAsset(const std::string &iso, const AssetEntity &asset) {
			return doTransaction([&iso, &asset, this]() {
				AssetEntity existAsset;

				if (SelectAsset(iso, asset.AssetID, existAsset)) {
					UpdateAsset(iso, asset);
				} else {
					InsertAsset(iso, asset);
				}
			});
		}

		bool AssetDataStore::PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets) {
			return doTransaction([&iso, &assets, this]() {
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
			return doTransaction([&iso, &assetID, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
					<< ASSET_TABLE_NAME
					<< " WHERE " << ASSET_COLUMN_ID << " = '" << assetID << "'"
					<< " AND " << ASSET_ISO << " = '" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool AssetDataStore::DeleteAllAssets(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM "
					<< ASSET_TABLE_NAME
					<< " WHERE " << ASSET_ISO << " = '" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool AssetDataStore::GetAssetDetails(const std::string &iso, const std::string &assetID, AssetEntity &asset) const {
			bool found = false;

			doTransaction([&iso, &assetID, &asset, &found, this]() {
				found = SelectAsset(iso, assetID, asset);
			});

			return found;
		}

		std::vector<AssetEntity> AssetDataStore::GetAllAssets(const std::string &iso) const {
			std::vector<AssetEntity> assets;

			doTransaction([&iso, &assets, this]() {
				AssetEntity asset;
				std::stringstream ss;

				ss << "SELECT "
					<< ASSET_COLUMN_ID << ", "
					<< ASSET_AMOUNT    << ", "
					<< ASSET_BUFF      << ", "
					<< ASSET_TXHASH
					<< " FROM " << ASSET_TABLE_NAME
					<< " WHERE " << ASSET_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->step(stmt)) {

					asset.AssetID = _sqlite->columnText(stmt, 0);
					asset.Amount = (uint64_t)_sqlite->columnInt64(stmt, 1);

					const uint8_t *pdata = (const uint8_t *) _sqlite->columnBlob(stmt, 2);
					size_t len = (size_t) _sqlite->columnBytes(stmt, 2);

#ifdef NDEBUG
					CMBlock buff;
					buff.Resize(len);
					memcpy(buff, pdata, len);
					entity.Asset = buff;
#else
					std::string str((const char *) pdata, len);
					asset.Asset = Utils::decodeHex(str);
#endif
					asset.TxHash = _sqlite->columnText(stmt, 3);

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
				<< ASSET_BUFF   << ", "
				<< ASSET_TXHASH
				<< " FROM " << ASSET_TABLE_NAME
				<< " WHERE " << ASSET_ISO << " = '" << iso << "' "
				<< " AND " << ASSET_COLUMN_ID << " = '" << assetID << "';";

			sqlite3_stmt *stmt;
			ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			while (SQLITE_ROW == _sqlite->step(stmt)) {
				found = true;

				asset.AssetID = assetID;
				asset.Amount = (uint64_t)_sqlite->columnInt64(stmt, 0);

				const uint8_t *pdata = (const uint8_t *) _sqlite->columnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->columnBytes(stmt, 1);

#ifdef NDEBUG
				CMBlock buff;
					buff.Resize(len);
					memcpy(buff, pdata, len);
					entity.Asset = buff;
#else
				std::string str((const char *) pdata, len);
				asset.Asset = Utils::decodeHex(str);
#endif
				asset.TxHash = _sqlite->columnText(stmt, 2);
			}

			return found;
		}

		bool AssetDataStore::InsertAsset(const std::string &iso, const AssetEntity &asset) {
			std::stringstream ss;

			ss << "INSERT INTO " << ASSET_TABLE_NAME << "("
				<< ASSET_COLUMN_ID << ","
				<< ASSET_AMOUNT    << ","
				<< ASSET_BUFF      << ","
				<< ASSET_TXHASH    << ","
				<< ASSET_ISO
				<< ") VALUES (?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			_sqlite->bindText(stmt, 1, asset.AssetID, nullptr);
			_sqlite->bindInt64(stmt, 2, asset.Amount);
#ifdef NDEBUG
			_sqlite->bindBlob(stmt, 3, asset.Asset, nullptr);
#else
			std::string str = Utils::encodeHex(asset.Asset);
			CMBlock bytes;
			bytes.SetMemFixed((const uint8_t *) str.c_str(), str.length());
			_sqlite->bindBlob(stmt, 3, bytes, nullptr);
#endif
			_sqlite->bindText(stmt, 4, asset.TxHash, nullptr);
			_sqlite->bindText(stmt, 5, iso, nullptr);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return true;
		}

		bool AssetDataStore::UpdateAsset(const std::string &iso, const AssetEntity &asset) {
			std::stringstream ss;

			ss << "UPDATE " << ASSET_TABLE_NAME << " SET "
				<< ASSET_AMOUNT << " = ?, "
				<< ASSET_BUFF   << " = ?, "
				<< ASSET_TXHASH << " = ? "
				<< " WHERE " << ASSET_ISO << " = '" << iso << "'"
				<< " AND " << ASSET_COLUMN_ID << " = '" << asset.AssetID << "';";

			sqlite3_stmt *stmt;
			ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			_sqlite->bindInt64(stmt, 1, asset.Amount);
#ifdef NDEBUG
			_sqlite->bindBlob(stmt, 2, asset.Asset, nullptr);
#else
			std::string str = Utils::encodeHex(asset.Asset);
			CMBlock bytes;
			bytes.SetMemFixed((const uint8_t *) str.c_str(), str.length());
			_sqlite->bindBlob(stmt, 2, bytes, nullptr);
#endif
			_sqlite->bindText(stmt, 3, asset.TxHash, nullptr);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return true;
		}

	}
}
