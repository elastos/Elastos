// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionDataStore.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Common/uint256.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Registry.h>

#include <string>

namespace Elastos {
	namespace ElaWallet {

		TransactionDataStore::TransactionDataStore(Sqlite *sqlite) : TableBase(sqlite) {
			InitializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			InitializeTable(TX_DATABASE_CREATE);
		}

		TransactionDataStore::~TransactionDataStore() {}

		bool TransactionDataStore::PutTransactionInternal(const std::string &iso, const TransactionPtr &tx) {
			std::string sql, txHash;

			sql = "INSERT INTO " + TX_TABLE_NAME + "(" +
				  TX_COLUMN_ID + "," +
				  TX_BUFF + "," +
				  TX_BLOCK_HEIGHT + "," +
				  TX_TIME_STAMP + "," +
				  TX_REMARK + "," +
				  TX_ASSETID + "," +
				  TX_ISO + ") VALUES (?, ?, ?, ?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			ByteStream stream;
			tx->Serialize(stream, true);

			txHash = tx->GetHash().GetHex();
			if (!_sqlite->BindText(stmt, 1, txHash, nullptr) ||
				!_sqlite->BindBlob(stmt, 2, stream.GetBytes(), nullptr) ||
				!_sqlite->BindInt(stmt, 3, tx->GetBlockHeight()) ||
				!_sqlite->BindInt64(stmt, 4, tx->GetTimestamp()) ||
				!_sqlite->BindText(stmt, 5, "", nullptr) ||
				!_sqlite->BindText(stmt, 6, "", nullptr) ||
				!_sqlite->BindText(stmt, 7, iso, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx put finalize");
				return false;
			}

			return true;
		}

		bool TransactionDataStore::PutTransaction(const std::string &iso, const TransactionPtr &tx) {
			std::string txHash = tx->GetHash().GetHex();
			if (ContainHash(txHash)) {
				Log::error("should not put in existed tx {}", tx->GetHash().GetHex());
				return false;
			}

			return DoTransaction([&iso, &tx, this]() { return this->PutTransactionInternal(iso, tx); });
		}

		bool TransactionDataStore::PutTransactions(const std::string &iso, const std::vector<TransactionPtr> &txns) {
			if (txns.empty())
				return true;

			return DoTransaction([&iso, &txns, this]() {
				for (size_t i = 0; i < txns.size(); ++i) {
					if (!this->PutTransactionInternal(iso, txns[i]))
						return false;
				}

				return true;
			});
		}

		bool TransactionDataStore::DeleteAllTransactions() {
			return DoTransaction([this]() {
				std::string sql;

				sql = "DELETE FROM " + TX_TABLE_NAME + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
		}

		size_t TransactionDataStore::GetAllTransactionsCount() const {
			size_t count = 0;

			std::string sql;

			sql = "SELECT COUNT(" + TX_COLUMN_ID + ") AS nums FROM " + TX_TABLE_NAME + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return 0;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all count finalize");
				return 0;
			}

			return count;
		}

		TransactionPtr TransactionDataStore::GetTransaction(const uint256 &hash, const std::string &chainID) {
			return SelectTxByHash(hash.GetHex(), chainID);
		}

		std::vector<TransactionPtr> TransactionDataStore::GetAllTransactions(const std::string &chainID) const {
			std::vector<TransactionPtr> txns;
			std::string sql;
			int r;

			sql = "SELECT " +
				  TX_COLUMN_ID + "," +
				  TX_BUFF + "," +
				  TX_BLOCK_HEIGHT + "," +
				  TX_TIME_STAMP + "," +
				  TX_ISO +
				  " FROM " + TX_TABLE_NAME + /*" ORDER BY " + TX_BLOCK_HEIGHT + " ASC*/";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
			}

			while (SQLITE_ROW == (r = _sqlite->Step(stmt))) {
				TransactionPtr tx;
				if (chainID == CHAINID_MAINCHAIN) {
					tx = TransactionPtr(new Transaction());
				} else if (chainID == CHAINID_IDCHAIN || chainID == CHAINID_TOKENCHAIN) {
					tx = TransactionPtr(new IDTransaction());
				}

				uint256 txHash(_sqlite->ColumnText(stmt, 0));

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
				ByteStream stream(pdata, len);

				uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
				uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
				std::string iso = _sqlite->ColumnText(stmt, 4);

				if (iso == "ela") {
					tx->Deserialize(stream);
					assert(txHash == tx->GetHash());
				} else if (iso == "ela1") {
					tx->Deserialize(stream, true);
					tx->SetHash(txHash);
				}

				tx->SetBlockHeight(blockHeight);
				tx->SetTimestamp(timeStamp);

				txns.push_back(tx);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx get all finalize");
				return {};
			}

			return txns;
		}

		bool TransactionDataStore::UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight,
													 time_t timestamp) {
			return DoTransaction([&hashes, &blockHeight, &timestamp, this]() {
				std::string sql, hash;

				for (size_t i = 0; i < hashes.size(); ++i) {
					hash = hashes[i].GetHex();
					sql = "UPDATE " + TX_TABLE_NAME + " SET " +
						  TX_BLOCK_HEIGHT + " = ?, " +
						  TX_TIME_STAMP + " = ? " +
						  " WHERE " + TX_COLUMN_ID + " = ?;";

					sqlite3_stmt *stmt;
					if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
						Log::error("prepare sql: {}", sql);
						return false;
					}

					if (!_sqlite->BindInt(stmt, 1, blockHeight) ||
						!_sqlite->BindInt64(stmt, 2, timestamp) ||
						!_sqlite->BindText(stmt, 3, hash, nullptr)) {
						Log::error("bind args");
					}

					if (!_sqlite->Step(stmt)) {
						Log::error("step");
					}

					if (!_sqlite->Finalize(stmt)) {
						Log::error("Tx update finalize");
						return false;
					}
				}
				return true;
			});
		}

		bool TransactionDataStore::DeleteTxByHash(const uint256 &hash) {
			return DoTransaction([&hash, this]() {
				std::string sql;
				std::string hashString = hash.GetHex();

				sql = "DELETE FROM " + TX_TABLE_NAME + " WHERE " + TX_COLUMN_ID + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindText(stmt, 1, hashString, nullptr)) {
					Log::error("bind args");
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("Tx delete finalize");
					return false;
				}
				return true;
			});
		}

		bool TransactionDataStore::DeleteTxByHashes(const std::vector<uint256> &hashes) {
			if (hashes.empty())
				return true;

			return DoTransaction([&hashes, this]() {
				std::string sql, hash;
				for (size_t i = 0; i < hashes.size(); ++i) {
					hash = hashes[i].GetHex();
					sql = "DELETE FROM " + TX_TABLE_NAME +
						  " WHERE " + TX_COLUMN_ID + " = ?;";

					sqlite3_stmt *stmt;
					if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
						Log::error("prepare sql: {}" + sql);
						return false;
					}

					if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
						Log::error("bind args");
					}

					if (SQLITE_DONE != _sqlite->Step(stmt)) {
						Log::error("step");
					}

					if (!_sqlite->Finalize(stmt)) {
						Log::error("Tx delete hashes finalize");
						return false;
					}
				}

				return true;
			});
		}

		void TransactionDataStore::flush() { _sqlite->flush(); }

		TransactionPtr TransactionDataStore::SelectTxByHash(const std::string &hash, const std::string &chainID) const {
			TransactionPtr tx = nullptr;

			std::string sql;

			sql = "SELECT " +
				  TX_COLUMN_ID + "," +
				  TX_BUFF + "," +
				  TX_BLOCK_HEIGHT + "," +
				  TX_TIME_STAMP + "," +
				  TX_ISO +
				  " FROM " + TX_TABLE_NAME +
				  " WHERE " + TX_COLUMN_ID + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return nullptr;
			}

			if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				if (chainID == CHAINID_MAINCHAIN) {
					tx = TransactionPtr(new Transaction());
				} else if (chainID == CHAINID_IDCHAIN || chainID == CHAINID_TOKENCHAIN) {
					tx = TransactionPtr(new IDTransaction());
				}

				uint256 txHash(_sqlite->ColumnText(stmt, 0));

				const uint8_t *pdata = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = (size_t) _sqlite->ColumnBytes(stmt, 1);
				ByteStream stream(pdata, len);

				uint32_t blockHeight = (uint32_t) _sqlite->ColumnInt(stmt, 2);
				uint32_t timeStamp = (uint32_t) _sqlite->ColumnInt(stmt, 3);
				std::string iso = _sqlite->ColumnText(stmt, 4);

				if (iso == "ela") {
					tx->Deserialize(stream);
					assert(txHash == tx->GetHash());
				} else if (iso == "ela1") {
					tx->Deserialize(stream, true);
					tx->SetHash(txHash);
				}

				tx->SetBlockHeight(blockHeight);
				tx->SetTimestamp(timeStamp);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx select finalize");
				return nullptr;
			}

			return tx;
		}

		bool TransactionDataStore::ContainHash(const std::string &hash) const {
			bool contain = false;

			std::string sql;

			sql = "SELECT " +
				  TX_COLUMN_ID +
				  " FROM " + TX_TABLE_NAME +
				  " WHERE " + TX_COLUMN_ID + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindText(stmt, 1, hash, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				contain = true;
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Tx contain finalize");
				return false;
			}

			return contain;
		}

	} // namespace ElaWallet
} // namespace Elastos
