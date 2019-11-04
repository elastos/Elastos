// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockDataSource.h"

#include <Common/Log.h>
#include <Plugin/Registry.h>
#include <Common/ByteStream.h>
#include <Common/ErrorChecker.h>
#include <Plugin/Interface/IMerkleBlock.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			TableBase(sqlite) {
			InitializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::MerkleBlockDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			InitializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::PutMerkleBlock(const std::string &iso, const MerkleBlockPtr &blockPtr) {
			return DoTransaction([&iso, &blockPtr, this]() {
				return this->PutMerkleBlockInternal(iso, blockPtr);
			});
		}

		bool MerkleBlockDataSource::PutMerkleBlocks(const std::string &iso,
													const std::vector<MerkleBlockPtr> &blocks) {
			if (blocks.empty())
				return true;

			return DoTransaction([&iso, &blocks, this]() {
				for (size_t i = 0; i < blocks.size(); ++i) {
					if (blocks[i]->GetHeight() > 0) {
						if (!this->PutMerkleBlockInternal(iso, blocks[i]))
							return false;
					}
				}

				return true;
			});
		}

		bool
		MerkleBlockDataSource::PutMerkleBlockInternal(const std::string &iso, const MerkleBlockPtr &blockPtr) {
			std::string sql;

			sql = "INSERT INTO " + MB_TABLE_NAME + " (" + MB_BUFF + "," + MB_HEIGHT + "," +
				  MB_ISO + ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			ByteStream stream;
			blockPtr->Serialize(stream);
			if (!_sqlite->BindBlob(stmt, 1, stream.GetBytes(), nullptr) ||
				!_sqlite->BindInt(stmt, 2, blockPtr->GetHeight()) ||
				!_sqlite->BindText(stmt, 3, iso, nullptr)) {
				Log::error("bind args");
				return false;
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
				return false;
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("finalize");
				return false;
			}

			return true;
		}

		bool MerkleBlockDataSource::DeleteMerkleBlock(const std::string &iso, long id) {
			return DoTransaction([&iso, &id, this]() {
				std::string sql;

				sql = "DELETE FROM " + MB_TABLE_NAME + " WHERE " + MB_COLUMN_ID + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindInt64(stmt, 1, id)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("finalize");
					return false;
				}

				return true;
			});
		}

		bool MerkleBlockDataSource::DeleteAllBlocks(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::string sql;

				sql = "DELETE FROM " + MB_TABLE_NAME + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}" + sql);
					return false;
				}

				return true;
			});
		}

		std::vector<MerkleBlockPtr> MerkleBlockDataSource::GetAllMerkleBlocks(const std::string &iso,
																			  const std::string &chainID) const {
			std::vector<MerkleBlockPtr> merkleBlocks;

			DoTransaction([&iso, &chainID, &merkleBlocks, this]() {

				std::string sql;
				sql = "SELECT " + MB_COLUMN_ID + ", " + MB_BUFF + ", " + MB_HEIGHT + " FROM " + MB_TABLE_NAME + ";";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}" + sql);
					return false;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					MerkleBlockPtr merkleBlock(Registry::Instance()->CreateMerkleBlock(chainID));
					// blockBytes
					const uint8_t *pblob = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = _sqlite->ColumnBytes(stmt, 1);
					ByteStream stream(pblob, len);
					merkleBlock->Deserialize(stream);

					// blockHeight
					uint32_t blockHeight = _sqlite->ColumnInt(stmt, 2);
					merkleBlock->SetHeight(blockHeight);

					merkleBlocks.push_back(merkleBlock);
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("finalize");
					return false;
				}

				return true;
			});

			return merkleBlocks;
		}

		void MerkleBlockDataSource::flush() {
			_sqlite->flush();
		}

	}
}

