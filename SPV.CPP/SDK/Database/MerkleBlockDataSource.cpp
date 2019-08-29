// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockDataSource.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Plugin/Interface/IMerkleBlock.h>
#include <SDK/Plugin/Registry.h>

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
				this->PutMerkleBlockInternal(iso, blockPtr);
			});
		}

		bool MerkleBlockDataSource::PutMerkleBlocks(const std::string &iso,
													const std::vector<MerkleBlockPtr> &blocks) {
			if (blocks.empty())
				return true;

			return DoTransaction([&iso, &blocks, this]() {
				for (size_t i = 0; i < blocks.size(); ++i) {
					if (blocks[i]->GetHeight() > 0) {
						this->PutMerkleBlockInternal(iso, blocks[i]);
					}
				}
			});
		}

		bool
		MerkleBlockDataSource::PutMerkleBlockInternal(const std::string &iso, const MerkleBlockPtr &blockPtr) {
			std::string sql;

			sql = "INSERT INTO " + MB_TABLE_NAME + " (" + MB_BUFF + "," + MB_HEIGHT + "," + MB_ISO + ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
										 "prepare sql " + sql);

			ByteStream stream;
			blockPtr->Serialize(stream);
			_sqlite->BindBlob(stmt, 1, stream.GetBytes(), nullptr);
			_sqlite->BindInt(stmt, 2, blockPtr->GetHeight());
			_sqlite->BindText(stmt, 3, iso, nullptr);

			_sqlite->Step(stmt);
			_sqlite->Finalize(stmt);
			return true;
		}

		bool MerkleBlockDataSource::DeleteMerkleBlock(const std::string &iso, long id) {
			return DoTransaction([&iso, &id, this]() {
				std::string sql;

				sql = "DELETE FROM " + MB_TABLE_NAME + " WHERE " + MB_COLUMN_ID + " = " + std::to_string(id) + ";";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "exec sql " + sql);
			});
		}

		bool MerkleBlockDataSource::DeleteAllBlocks(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::string sql;

				sql = "DELETE FROM " + MB_TABLE_NAME + ";";

				ErrorChecker::CheckCondition(!_sqlite->exec(sql, nullptr, nullptr), Error::SqliteError,
											 "exec sql " + sql);
			});
		}

		std::vector<MerkleBlockPtr> MerkleBlockDataSource::GetAllMerkleBlocks(const std::string &iso,
		                                                                      const std::string &pluginType) const {
			std::vector<MerkleBlockPtr> merkleBlocks;

			DoTransaction([&iso, &pluginType, &merkleBlocks, this]() {

				std::string sql;
				sql = "SELECT " + MB_COLUMN_ID + ", " + MB_BUFF + ", " + MB_HEIGHT + " FROM " + MB_TABLE_NAME + ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(sql, &stmt, nullptr), Error::SqliteError,
											 "prepare sql " + sql);

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					MerkleBlockPtr merkleBlock(Registry::Instance()->CreateMerkleBlock(pluginType));
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

				_sqlite->Finalize(stmt);
			});

			return merkleBlocks;
		}

		void MerkleBlockDataSource::flush() {
			_sqlite->flush();
		}

	}
}

