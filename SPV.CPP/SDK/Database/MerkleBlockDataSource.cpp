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

#include "MerkleBlockDataSource.h"

#include <Common/Log.h>
#include <Plugin/Registry.h>
#include <Common/ByteStream.h>
#include <Plugin/Interface/IMerkleBlock.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

#define MERKLEBLOCK_ISO "ela2"
		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		void MerkleBlockDataSource::InitializeTable() {
			TableBase::InitializeTable(MB_DATABASE_CREATE);
		}

		bool MerkleBlockDataSource::PutMerkleBlock(const MerkleBlockPtr &blockPtr) {
			return DoTransaction([&blockPtr, this]() {
				return this->PutMerkleBlockInternal(blockPtr);
			});
		}

		bool MerkleBlockDataSource::PutMerkleBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			if (blocks.empty())
				return true;

			return DoTransaction([&replace, &blocks, this]() {
				if (replace) {
					std::string sql = "DELETE FROM " + MB_TABLE_NAME + ";";

					if (!_sqlite->exec(sql, nullptr, nullptr)) {
						Log::error("exec sql: {}" + sql);
						return false;
					}
				}

				for (size_t i = 0; i < blocks.size(); ++i) {
					if (blocks[i]->GetHeight() > 0) {
						if (!this->PutMerkleBlockInternal(blocks[i]))
							return false;
					}
				}

				return true;
			});
		}

		bool
		MerkleBlockDataSource::PutMerkleBlockInternal(const MerkleBlockPtr &blockPtr) {
			std::string sql;

			sql = "INSERT INTO " + MB_TABLE_NAME + " (" + MB_BUFF + "," + MB_HEIGHT + "," +
				  MB_ISO + ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			ByteStream stream;
			blockPtr->Serialize(stream, MERKLEBLOCK_VERSION_1);
			if (!_sqlite->BindBlob(stmt, 1, stream.GetBytes(), nullptr) ||
				!_sqlite->BindInt(stmt, 2, blockPtr->GetHeight()) ||
				!_sqlite->BindText(stmt, 3, MERKLEBLOCK_ISO, nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("mb put finalize");
				return false;
			}

			return true;
		}

		bool MerkleBlockDataSource::DeleteMerkleBlock(long id) {
			return DoTransaction([&id, this]() {
				std::string sql;

				sql = "DELETE FROM " + MB_TABLE_NAME + " WHERE " + MB_COLUMN_ID + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindInt64(stmt, 1, id)) {
					Log::error("bind args");
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("mb delete finalize");
					return false;
				}

				return true;
			});
		}

		bool MerkleBlockDataSource::DeleteAllBlocks() {
			return DeleteAll(MB_TABLE_NAME);
		}

		std::vector<MerkleBlockPtr> MerkleBlockDataSource::GetAllMerkleBlocks(const std::string &chainID) const {
			std::vector<MerkleBlockPtr> merkleBlocks;

			std::string sql;
			sql = "SELECT " + MB_COLUMN_ID + "," + MB_BUFF + "," + MB_HEIGHT + "," + MB_ISO + " FROM " + MB_TABLE_NAME + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return {};
			}

			while (SQLITE_ROW == _sqlite->Step(stmt)) {
				// blockBytes
				const uint8_t *pblob = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
				size_t len = _sqlite->ColumnBytes(stmt, 1);
				// blockHeight
				uint32_t blockHeight = _sqlite->ColumnInt(stmt, 2);
				std::string iso = _sqlite->ColumnText(stmt, 3);

				MerkleBlockPtr merkleBlock(Registry::Instance()->CreateMerkleBlock(chainID));
				ByteStream stream(pblob, len);
				if (iso == MERKLEBLOCK_ISO)
					merkleBlock->Deserialize(stream, MERKLEBLOCK_VERSION_1);
				else
					merkleBlock->Deserialize(stream, MERKLEBLOCK_VERSION_0);

				merkleBlock->SetHeight(blockHeight);
				merkleBlocks.push_back(merkleBlock);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("mb get all finalize");
				return {};
			}

			return merkleBlocks;
		}

	}
}

