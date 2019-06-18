// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ErrorChecker.h>
#include "CoinBaseUTXODataStore.h"

namespace Elastos {
	namespace ElaWallet {

		CoinBaseUTXOEntity::CoinBaseUTXOEntity() :
			_spent(false),
			_index(0),
			_outputLock(0),
			_blockHeight(0),
			_timestamp(0) {
		}

		CoinBaseUTXOEntity::~CoinBaseUTXOEntity() {
		}

		bool CoinBaseUTXOEntity::Spent() const {
			return _spent;
		}

		void CoinBaseUTXOEntity::SetSpent(bool status) {
			_spent = status;
		}

		const uint16_t &CoinBaseUTXOEntity::Index() const {
			return _index;
		}

		void CoinBaseUTXOEntity::SetIndex(uint16_t index) {
			_index = index;
		}

		const uint168 &CoinBaseUTXOEntity::ProgramHash() const {
			return _programHash;
		}

		void CoinBaseUTXOEntity::SetProgramHash(const uint168 &hash) {
			_programHash = hash;
		}

		const uint256 &CoinBaseUTXOEntity::AssetID() const {
			return _assetID;
		}

		void CoinBaseUTXOEntity::SetAssetID(const uint256 &ID) {
			_assetID = ID;
		}

		const uint32_t &CoinBaseUTXOEntity::OutputLock() const {
			return _outputLock;
		}

		void CoinBaseUTXOEntity::SetOutputLock(uint32_t outputLock) {
			_outputLock = outputLock;
		}

		const BigInt &CoinBaseUTXOEntity::Amount() const {
			return _amount;
		}

		void CoinBaseUTXOEntity::SetAmount(const BigInt &amount) {
			_amount = amount;
		}

		const bytes_ptr &CoinBaseUTXOEntity::Payload() const {
			return _payload;
		}

		void CoinBaseUTXOEntity::SetPayload(const bytes_ptr &payload) {
			_payload = payload;
		}

		const time_t &CoinBaseUTXOEntity::Timestamp() const {
			return _timestamp;
		}

		void CoinBaseUTXOEntity::SetTimestamp(time_t timestamp) {
			_timestamp = timestamp;
		}

		const uint32_t &CoinBaseUTXOEntity::BlockHeight() const {
			return _blockHeight;
		}

		void CoinBaseUTXOEntity::SetBlockHeight(uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		const std::string &CoinBaseUTXOEntity::TxHash() const {
			return _txHash;
		}

		void CoinBaseUTXOEntity::SetTxHash(const std::string &txHash) {
			_txHash = txHash;
		}


		CoinBaseUTXODataStore::CoinBaseUTXODataStore(Sqlite *sqlite) :
			TableBase(sqlite) {
			InitializeTable(_databaseCreate);
		}

		CoinBaseUTXODataStore::~CoinBaseUTXODataStore() {

		}

		bool CoinBaseUTXODataStore::Put(const std::vector<CoinBaseUTXOEntity> &entitys) {
			return DoTransaction([&entitys, this]() {
				for (size_t i = 0; i < entitys.size(); ++i) {
					this->PutInternal(entitys[i]);
				}
			});
		}

		bool CoinBaseUTXODataStore::Put(const CoinBaseUTXOEntity &entity) {
			return DoTransaction([&entity, this]() {
				this->PutInternal(entity);
			});
		}

		bool CoinBaseUTXODataStore::DeleteAll() {
			return DoTransaction([this]() {
				std::string sql;

				sql = "DELETE FROM "  + _tableName + "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		size_t CoinBaseUTXODataStore::GetTotalCount() const {
			size_t count = 0;

			DoTransaction([&count, this]() {
				std::string sql;

				sql = std::string("SELECT ") + "COUNT(" + _txHash + ") AS nums "
				   + "FROM " + _tableName + ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}

				_sqlite->Finalize(stmt);
			});

			return count;
		}

		std::vector<CoinBaseUTXOEntityPtr> CoinBaseUTXODataStore::GetAll() const {
			std::vector<CoinBaseUTXOEntityPtr> entitys;

			DoTransaction([&entitys, this]() {
				std::string sql;

				sql = "SELECT "
				   + _txHash + ", "
				   + _blockHeight + ", "
				   + _timestamp + ", "
				   + _index + ", "
				   + _programHash + ", "
				   + _assetID + ", "
				   + _outputLock + ", "
				   + _amount + ", "
				   + _payload + ", "
				   + _spent
				   + " FROM " + _tableName + ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					CoinBaseUTXOEntityPtr entity(new CoinBaseUTXOEntity());

					entity->SetTxHash(_sqlite->ColumnText(stmt, 0));
					entity->SetBlockHeight((uint32_t) _sqlite->ColumnInt(stmt, 1));
					entity->SetTimestamp((time_t) _sqlite->ColumnInt64(stmt, 2));
					entity->SetIndex((uint16_t) _sqlite->ColumnInt(stmt, 3));
					entity->SetProgramHash(uint168(*_sqlite->ColumnBlobBytes(stmt, 4)));
					entity->SetAssetID(uint256(*_sqlite->ColumnBlobBytes(stmt, 5)));
					entity->SetOutputLock((uint32_t) _sqlite->ColumnInt(stmt, 6));
					BigInt amount;
					amount.setDec(_sqlite->ColumnText(stmt, 7));
					entity->SetAmount(amount);
					entity->SetPayload(_sqlite->ColumnBlobBytes(stmt, 8));
					entity->SetSpent(_sqlite->ColumnInt(stmt, 9) != 0);

					entitys.push_back(entity);
				}

				_sqlite->Finalize(stmt);
			});

			return entitys;
		}

		bool CoinBaseUTXODataStore::Update(const std::vector<uint256> &txHashes, uint32_t blockHeight,
										   time_t timestamp) {
			return DoTransaction([&txHashes, &blockHeight, &timestamp, this]() {
				std::string sql;

				for (size_t i = 0; i < txHashes.size(); ++i) {
					sql = "UPDATE " + _tableName + " SET " + _blockHeight + " = ?, " +
						_timestamp + " = ? " + " WHERE " + _txHash + " = '" + txHashes[i].GetHex() + "';";

					sqlite3_stmt *stmt;
					ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + sql);

					_sqlite->BindInt(stmt, 1, blockHeight);
					_sqlite->BindInt64(stmt, 2, timestamp);

					_sqlite->Step(stmt);

					_sqlite->Finalize(stmt);
				}
			});
		}

		bool CoinBaseUTXODataStore::UpdateSpent(const std::vector<uint256> &txHashes) {
			return DoTransaction([&txHashes, this]() {
				std::string sql;

				for (size_t i = 0; i < txHashes.size(); ++i) {
					sql = "UPDATE " + _tableName + " SET " + _spent + " = ? " + " WHERE " +
						_txHash + " = '" + txHashes[i].GetHex() + "';";

					sqlite3_stmt *stmt;
					ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
												 "Prepare sql " + sql);

					_sqlite->BindInt(stmt, 1, 1);

					_sqlite->Step(stmt);

					_sqlite->Finalize(stmt);
				}
			});
		}

		bool CoinBaseUTXODataStore::Delete(const std::string &hash) {
			return DoTransaction([&hash, this]() {
				std::string sql;

				sql = "DELETE FROM " + _tableName + " WHERE " + _txHash + " = '" + hash + "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + sql);
			});
		}

		bool CoinBaseUTXODataStore::PutInternal(const CoinBaseUTXOEntity &entity) {
			std::string sql;

			sql = "INSERT INTO " + _tableName + "("
			   + _txHash + ","
			   + _blockHeight + ","
			   + _timestamp + ","
			   + _index + ","
			   + _programHash + ","
			   + _assetID + ","
			   + _outputLock + ","
			   + _amount + ","
			   + _payload + ","
			   + _spent
			   + ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + sql);

			_sqlite->BindText(stmt, 1, entity.TxHash(), nullptr);
			_sqlite->BindInt(stmt, 2, entity.BlockHeight());
			_sqlite->BindInt64(stmt, 3, entity.Timestamp());
			_sqlite->BindInt(stmt, 4, entity.Index());
			_sqlite->BindBlob(stmt, 5, entity.ProgramHash().bytes(), nullptr);
			_sqlite->BindBlob(stmt, 6, entity.AssetID().begin(), entity.AssetID().size(), nullptr);
			_sqlite->BindInt(stmt, 7, entity.OutputLock());
			_sqlite->BindText(stmt, 8, entity.Amount().getDec(), nullptr);
			if (entity.Payload())
				_sqlite->BindBlob(stmt, 9, *entity.Payload(), nullptr);
			else
				_sqlite->BindBlob(stmt, 9, nullptr, 0, nullptr);
			_sqlite->BindInt(stmt, 10, entity.Spent());

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);

			return true;
		}

	}
}