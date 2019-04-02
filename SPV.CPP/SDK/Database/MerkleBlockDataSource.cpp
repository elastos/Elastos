// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockDataSource.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>

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

		bool MerkleBlockDataSource::PutMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return DoTransaction([&iso, &blockEntity, this]() {
				this->PutMerkleBlockInternal(iso, blockEntity);
			});
		}

		bool MerkleBlockDataSource::PutMerkleBlocks(const std::string &iso,
													const std::vector<MerkleBlockEntity> &blockEntities) {
			return DoTransaction([&iso, &blockEntities, this]() {
				for (size_t i = 0; i < blockEntities.size(); ++i) {
					this->PutMerkleBlockInternal(iso, blockEntities[i]);
				}
			});
		}

		bool
		MerkleBlockDataSource::PutMerkleBlockInternal(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << MB_TABLE_NAME << " (" <<
			   MB_BUFF << "," <<
			   MB_HEIGHT << "," <<
			   MB_ISO <<
			   ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "prepare sql " + ss.str());
			_sqlite->BindBlob(stmt, 1, blockEntity.blockBytes, nullptr);
			_sqlite->BindInt(stmt, 2, blockEntity.blockHeight);
			_sqlite->BindText(stmt, 3, iso, nullptr);

			_sqlite->Step(stmt);
			_sqlite->Finalize(stmt);
			return true;
		}

		bool MerkleBlockDataSource::DeleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return DoTransaction([&iso, &blockEntity, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_COLUMN_ID << " = " << blockEntity.id <<
				   " AND " << MB_ISO << " = " << "'" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool MerkleBlockDataSource::DeleteAllBlocks(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_ISO << " = '" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::GetAllMerkleBlocks(const std::string &iso) const {
			std::vector<MerkleBlockEntity> merkleBlocks;

			DoTransaction([&iso, &merkleBlocks, this]() {
				MerkleBlockEntity merkleBlock;
				std::stringstream ss;
				ss << "SELECT " <<
				   MB_COLUMN_ID << ", " <<
				   MB_BUFF << ", " <<
				   MB_HEIGHT <<
				   " FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					// id
					merkleBlock.id = _sqlite->ColumnInt(stmt, 0);

					// blockBytes
					const uint8_t *pblob = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = _sqlite->ColumnBytes(stmt, 1);

					merkleBlock.blockBytes.assign(pblob, pblob + len);

					// blockHeight
					merkleBlock.blockHeight = _sqlite->ColumnInt(stmt, 2);

					merkleBlocks.push_back(merkleBlock);
				}

				_sqlite->Finalize(stmt);
			});

			return merkleBlocks;
		}

	}
}

